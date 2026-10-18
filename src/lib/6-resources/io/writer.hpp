/* @DOC Buffered output to a file descriptor.

   Performance:
   - Layout: three pointers (buffer begin, fill cursor, end), the fd, the policy and the sticky
     error; 40 bytes on 64-bit. The buffer is borrowed, never allocated: the caller (or the
     per-thread standard writers) owns it.
   - Formatting writes straight into the buffer through window()/drain()/settle(): no staging
     copy. write() is one bounds check and a memcpy while the bytes fit.
   - Slow paths (writing out, writes larger than the buffer) are out of line and take the fd
     and pointers, never `this`, so callers keep the cursor in a register.
   - Writes larger than the whole buffer bypass it: pending bytes first, then the data
     directly, two write calls and no copy.
   - line policy scans only the bytes of the current message for '\n' (memchr).
   - A writer without a buffer (unbuffered(fd)) has an empty window: print stages each
     message on its stack (1 KiB) and writes it with one call.
   - A failed write-out moves the bytes not written to the front of the buffer (memmove),
     error path only. */
#ifndef ca_resources_io_writer_hpp
#define ca_resources_io_writer_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/find_byte.hpp"
#include "1-primitives/memory/move_bytes.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "6-resources/io/buffering.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- WRITER ---------------------------------------------------------

		namespace impl {

			template <typename D>
			concept has_native_descriptor = requires (const D& file) {
				{ file.native() } -> ca::is_same<int>;
			};

		} // namespace impl


		/* @DOC Writer over a file descriptor it does not own, buffering into memory the caller
		   provides, with a policy (ca::io::buffering). Each call returns its own error, and the
		   first one since the last check is kept, sticky: error() peeks, clear_error() forgets it
		   once handled, flush() returns and clears it. When writing out fails, the bytes not
		   written stay pending, never retried behind the caller's back: flush() retries them,
		   discard() drops them; a message still being formatted is dropped whole unless part of
		   it already went out. Cleanup is flush(); the destructor is a failsafe that writes out
		   pending bytes silently. Not thread-safe: one writer per thread (ca::io::out() is). */
		class writer final {


			private:

				// -- private types ------------------------------------------

				using self = ca::io::writer;


				// -- private members ----------------------------------------

				char* _begin;
				char* _at;
				char* _end;
				int _fd;
				ca::io::buffering _mode;
				ca::error _error;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC Closed: every write fails with errc::bad_descriptor. */
				constexpr writer(void) noexcept
				: _begin{nullptr}, _at{nullptr}, _end{nullptr}, _fd{-1}, _mode{ca::io::buffering::none}, _error{} {
				}

				/* @DOC Over fd, buffering into buffer (a char array, or ca::span{data, capacity}), which
				   must outlive the writer. An empty buffer makes it unbuffered whatever the policy. */
				constexpr writer(const int fd, const ca::io::buffering mode, const ca::span<char> buffer) noexcept
				: _begin{buffer.begin()}, _at{buffer.begin()}, _end{buffer.end()}, _fd{fd}, _mode{mode}, _error{} {
				}

				/* @DOC Over file's descriptor (a ca::fs::descriptor, or anything with native() -> int),
				   borrowed: file must stay open while the writer writes. */
				template <ca::io::impl::has_native_descriptor D>
				constexpr writer(const D& file, const ca::io::buffering mode, const ca::span<char> buffer) noexcept
				: _begin{buffer.begin()}, _at{buffer.begin()}, _end{buffer.end()}, _fd{file.native()}, _mode{mode}, _error{} {
				}

				/* @DOC No buffer: each message is one write (print stages it), each write() too. */
				[[nodiscard]] static auto unbuffered(const int fd) noexcept -> self {
					return self{fd, ca::io::buffering::none, ca::span<char>{}};
				}

				writer(const self&) = delete;

				constexpr writer(self&& other) noexcept
				: _begin{ca::exchange(other._begin, nullptr)}, _at{ca::exchange(other._at, nullptr)}, _end{ca::exchange(other._end, nullptr)}, _fd{ca::exchange(other._fd, -1)}, _mode{other._mode}, _error{other._error} {
				}

				/* @DOC Failsafe for a skipped flush(): writes pending bytes out, ignoring errors. */
				~writer(void) noexcept {

					const char* from = _begin;
					if (from != _at)
						static_cast<void>(self::_emit(_fd, from, _at));
				}


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				auto operator=(self&&) -> self& = delete;


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto native(void) const noexcept -> int {
					return _fd;
				}

				[[nodiscard]] constexpr auto buffering(void) const noexcept -> ca::io::buffering {
					return _mode;
				}

				/* @DOC The whole buffer; empty when unbuffered. */
				[[nodiscard]] constexpr auto buffer(void) const noexcept -> ca::span<char> {
					return ca::span<char>{_begin, static_cast<ca::usz>(_end - _begin)};
				}

				[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
					return static_cast<ca::usz>(_end - _begin);
				}

				/* @DOC Bytes buffered, not yet written out (including those a failed write-out kept). */
				[[nodiscard]] constexpr auto pending(void) const noexcept -> ca::usz {
					return static_cast<ca::usz>(_at - _begin);
				}

				/* @DOC The sticky error, ok when none; kept until flush() or clear_error(). */
				[[nodiscard]] constexpr auto error(void) const noexcept -> ca::error {
					return _error;
				}


				// -- methods ------------------------------------------------

				/* @DOC Appends data, then applies the policy to it; fails when that writes out and
				   the write fails, data then pending. Data that does not fit is refused when writing
				   out the pending bytes fails. Data larger than the buffer goes out directly: on
				   failure, what was not written is lost. */
				[[nodiscard]] auto write(const ca::span<const ca::byte> data) noexcept -> ca::result<void> {

					const char* const bytes = reinterpret_cast<const char*>(data.data());
					const ca::usz size = data.size();
					if (size > static_cast<ca::usz>(_end - _at)) [[unlikely]]
						return this->_write_large(bytes, size);
					ca::copy_bytes(_at, bytes, size);
					_at += size;
					if (_mode != ca::io::buffering::full)
						return this->_apply(bytes, size);
					return ca::result<void>{};
				}

				/* @DOC Writes pending bytes out; this write-out's error, also kept sticky. What a
				   reader tied to this writer calls before it blocks. */
				[[nodiscard]] auto write_pending(void) noexcept -> ca::result<void> {
					return self::_result(this->_write_out());
				}

				/* @DOC Writes pending bytes out, then returns the sticky error (this write-out's
				   when it is the first) and clears it. */
				[[nodiscard]] auto flush(void) noexcept -> ca::result<void> {

					static_cast<void>(this->_write_out());
					return self::_result(ca::exchange(_error, ca::error{}));
				}

				/* @DOC Forgets the sticky error, once handled, so flush() does not report it again. */
				constexpr auto clear_error(void) noexcept -> void {
					_error = ca::error{};
				}

				/* @DOC Drops the pending bytes, unwritten. */
				constexpr auto discard(void) noexcept -> void {
					_at = _begin;
				}

				/* @DOC Writes pending bytes out, then buffers into buffer instead (empty: unbuffered);
				   bytes a failed write-out kept move along when they fit, else are lost. Returns the
				   write-out's error. The old buffer is free once this returns. */
				[[nodiscard]] auto set_buffer(const ca::span<char> buffer) noexcept -> ca::result<void> {

					const ca::error error = this->_write_out();
					const ca::usz kept = this->pending() <= buffer.size() ? this->pending() : 0U;
					ca::move_bytes(buffer.begin(), _begin, kept);
					_begin = buffer.begin();
					   _at = buffer.begin() + kept;
					  _end = buffer.end();
					return self::_result(error);
				}

				/* @DOC Changes the policy; none writes pending bytes out now, returning that error. */
				[[nodiscard]] auto set_buffering(const ca::io::buffering mode) noexcept -> ca::result<void> {

					_mode = mode;
					if (mode == ca::io::buffering::none)
						return self::_result(this->_write_out());
					return ca::result<void>{};
				}

				/* @DOC Free buffer space; empty for an unbuffered writer. See is_windowed_writer. */
				[[nodiscard]] constexpr auto window(void) const noexcept -> ca::span<char> {
					return ca::span<char>{_at, static_cast<ca::usz>(_end - _at)};
				}

				/* @DOC Writes the buffer out up to at; lends it again whole. On failure the message is
				   dropped and the earlier bytes not written stay pending. */
				[[nodiscard]] auto drain(char* const at) noexcept -> ca::result<ca::span<char>> {

					const char* from = _begin;
					const ca::error error = self::_emit(_fd, from, at);
					if (!error.ok()) [[unlikely]] {
						this->_keep(from < _at ? from : _at);
						return this->_fail(error);
					}
					_at = _begin;
					return this->buffer();
				}

				/* @DOC Ends a message at at, then applies the policy to it; fails as write. */
				[[nodiscard]] auto settle(char* const at) noexcept -> ca::result<void> {

					const char* const from = _at;
					_at = at;
					if (_mode != ca::io::buffering::full)
						return this->_apply(from, static_cast<ca::usz>(at - from));
					return ca::result<void>{};
				}


			private:

				// -- private methods ----------------------------------------

				/* @DOC Writes [from, to) to fd, from advancing past what went out; the error, ok on
				   success. */
				[[nodiscard]] static auto _emit(int fd, const char*& from, const char* to) noexcept -> ca::error;

				[[nodiscard]] static constexpr auto _result(const ca::error error) noexcept -> ca::result<void> {

					if (!error.ok()) [[unlikely]]
						return ca::failure{error};
					return ca::result<void>{};
				}

				constexpr auto _record(const ca::error error) noexcept -> void {
					if (_error.ok())
						_error = error;
				}

				[[nodiscard]] constexpr auto _fail(const ca::error error) noexcept -> ca::failure<ca::error> {

					this->_record(error);
					return ca::failure{error};
				}

				/* @DOC Pending becomes [from, _at), moved to the front. */
				auto _keep(const char* const from) noexcept -> void {

					const ca::usz left = static_cast<ca::usz>(_at - from);
					ca::move_bytes(_begin, from, left);
					_at = _begin + left;
				}

				[[nodiscard]] auto _write_out(void) noexcept -> ca::error {

					const char* from = _begin;
					if (from == _at)
						return ca::error{};
					const ca::error error = self::_emit(_fd, from, _at);
					if (!error.ok()) [[unlikely]] {
						this->_keep(from);
						this->_record(error);
						return error;
					}
					_at = _begin;
					return error;
				}

				[[nodiscard]] auto _apply(const char* const added, const ca::usz size) noexcept -> ca::result<void> {

					if (_mode == ca::io::buffering::none || ca::find_byte(added, '\n', size) != nullptr)
						return self::_result(this->_write_out());
					return ca::result<void>{};
				}

				[[nodiscard]] auto _write_large(const char* const bytes, const ca::usz size) noexcept -> ca::result<void> {

					const ca::error flushed = this->_write_out();
					if (!flushed.ok()) [[unlikely]]
						return ca::failure{flushed};
					if (size >= this->capacity()) {
						const char* from = bytes;
						const ca::error error = self::_emit(_fd, from, bytes + size);
						if (!error.ok()) [[unlikely]]
							return this->_fail(error);
						return ca::result<void>{};
					}
					ca::copy_bytes(_at, bytes, size);
					_at += size;
					if (_mode != ca::io::buffering::full)
						return this->_apply(bytes, size);
					return ca::result<void>{};
				}

		}; // class writer

	} // namespace io

} // namespace ca

#endif // ca_resources_io_writer_hpp
