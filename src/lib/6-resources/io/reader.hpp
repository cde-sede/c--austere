/* @DOC Buffered input from a file descriptor.

   Performance:
   - Layout: four pointers (buffer begin, read cursor, end of data, end of buffer), the fd and
     a tied writer pointer; 48 bytes on 64-bit. The buffer is borrowed, never allocated.
     Trivially destructible: nothing to flush on input.
   - read() serving buffered bytes is one compare and a memcpy; reads at least as large as
     the buffer go straight to the fd, no copy.
   - fill()/consume() and read_until() lend the buffer: lines and tokens are parsed in place.
     read_until scans each byte once (memchr), across refills; it moves the unread tail to
     the front only when the buffer must take more.
   - One read call per refill, sized to the free space.

   Baseline stdio (getline, fread); a file of 1e6 lines of about 45 bytes:
   --BENCH bench/read.cpp
   bench/read.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | 1e6 lines in place, read_until | 0.63x | 0.64x | std: getline |
   | 1e6 lines copied, read_line | 0.88x | 0.93x | std: getline |
   | 4 KiB chunks, read | 0.55x | 0.53x | std: fread |
   BENCH-- */
#ifndef ca_resources_io_reader_hpp
#define ca_resources_io_reader_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/read.hpp"
#include "6-resources/io/writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- READER ---------------------------------------------------------

		/* @DOC Input over a file descriptor it does not own, buffering into memory the caller
		   provides (swappable with set_buffer). A buffered reader (ca::io::is_buffered_reader):
		   fill/consume and read_until read in place. Errors come back per call. A tied writer is
		   written out before each read that may block, so a prompt shows before input is
		   awaited; its failure does not stop the read, it stays in the writer's sticky error. Not
		   thread-safe. */
		class reader final {


			private:

				// -- private types ------------------------------------------

				using self = ca::io::reader;


				// -- private members ----------------------------------------

				char* _begin;
				char* _at;
				char* _filled;
				char* _end;
				ca::io::writer* _tie;
				int _fd;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC Closed: every read fails with errc::bad_descriptor. */
				constexpr reader(void) noexcept
				: _begin{nullptr}, _at{nullptr}, _filled{nullptr}, _end{nullptr}, _tie{nullptr}, _fd{-1} {
				}

				/* @DOC Over fd, buffering into buffer (a char array, or ca::span{data, capacity}), which
				   must outlive the reader. An empty buffer: unbuffered, read only (fill and read_until
				   fail). */
				constexpr reader(const int fd, const ca::span<char> buffer) noexcept
				: _begin{buffer.begin()}, _at{buffer.begin()}, _filled{buffer.begin()}, _end{buffer.end()}, _tie{nullptr}, _fd{fd} {
				}

				/* @DOC Over file's descriptor (a ca::fs::descriptor, or anything with native() -> int),
				   borrowed: file must stay open while the reader reads. */
				template <ca::io::impl::has_native_descriptor D>
				constexpr reader(const D& file, const ca::span<char> buffer) noexcept
				: self{file.native(), buffer} {
				}

				[[nodiscard]] static constexpr auto unbuffered(const int fd) noexcept -> self {
					return self{fd, ca::span<char>{}};
				}

				reader(const self&) = delete;

				constexpr reader(self&& other) noexcept
				: _begin{ca::exchange(other._begin, nullptr)}, _at{ca::exchange(other._at, nullptr)}, _filled{ca::exchange(other._filled, nullptr)}, _end{ca::exchange(other._end, nullptr)}, _tie{ca::exchange(other._tie, nullptr)}, _fd{ca::exchange(other._fd, -1)} {
				}


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				auto operator=(self&&) -> self& = delete;


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto native(void) const noexcept -> int {
					return _fd;
				}

				/* @DOC The whole buffer; empty when unbuffered. */
				[[nodiscard]] constexpr auto buffer(void) const noexcept -> ca::span<char> {
					return ca::span<char>{_begin, static_cast<ca::usz>(_end - _begin)};
				}

				[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
					return static_cast<ca::usz>(_end - _begin);
				}

				/* @DOC Bytes buffered and not consumed yet: what fill() returns without reading. */
				[[nodiscard]] constexpr auto available(void) const noexcept -> ca::usz {
					return static_cast<ca::usz>(_filled - _at);
				}

				[[nodiscard]] constexpr auto tied(void) const noexcept -> ca::io::writer* {
					return _tie;
				}


				// -- methods ------------------------------------------------

				/* @DOC Up to buffer.size() bytes (not 0): buffered ones first, else one read; 0 at end
				   of input. */
				[[nodiscard]] auto read(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {

					const ca::usz buffered = static_cast<ca::usz>(_filled - _at);
					if (buffered == 0U) [[unlikely]]
						return this->_read_empty(buffer);
					const ca::usz count = buffer.size() < buffered ? buffer.size() : buffered;
					ca::copy_bytes(buffer.data(), _at, count);
					_at += count;
					return count;
				}

				/* @DOC read until buffer is full or the end of input; how many. On failure the bytes
				   already taken are lost to the caller. */
				[[nodiscard]] auto read_full(ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz>;

				/* @DOC The buffered bytes, reading more first when there are none; empty at end of
				   input. Valid until the next call. Unbuffered: errc::invalid_argument. */
				[[nodiscard]] auto fill(void) noexcept -> ca::result<ca::sv::str> {
					if (_at == _filled) [[unlikely]] {
						auto got = this->_refill();
						if (!got) [[unlikely]]
							return got.propagate();
					}
					return ca::sv::str{_at, static_cast<ca::usz>(_filled - _at)};
				}

				/* @DOC Marks count buffered bytes (at most all) as read. */
				constexpr auto consume(const ca::usz count) noexcept -> void {
					_at += count < static_cast<ca::usz>(_filled - _at) ? count : static_cast<ca::usz>(_filled - _at);
				}

				/* @DOC The bytes up to and including the next delimiter, in place, valid until the next
				   call. Without the delimiter: the rest of input at its end (empty: nothing left), or
				   a full buffer when the run is longer than capacity(), the rest following on the
				   next call. Unbuffered: errc::invalid_argument. */
				[[nodiscard]] auto read_until(char delimiter) noexcept -> ca::result<ca::sv::str>;

				/* @DOC Moves the unread bytes to buffer and reads into it from now on; errc::no_space,
				   nothing changed, when they do not fit. The old buffer is free once this succeeds. */
				[[nodiscard]] auto set_buffer(ca::span<char> buffer) noexcept -> ca::result<void>;

				/* @DOC Writes writer's pending bytes out before each read that may block (null: none),
				   as C ties stdin to stdout. writer must outlive the tie. */
				constexpr auto tie(ca::io::writer* const writer) noexcept -> void {
					_tie = writer;
				}


			private:

				// -- private methods ----------------------------------------

				[[nodiscard]] auto _read_empty(ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz>;

				[[nodiscard]] auto _refill(void) noexcept -> ca::result<ca::usz>;

		}; // class reader

	} // namespace io

} // namespace ca

#endif // ca_resources_io_reader_hpp
