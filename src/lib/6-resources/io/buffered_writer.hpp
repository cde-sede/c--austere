/* @DOC Writer owning its buffer (and optionally its descriptor).

   Performance:
   - Layout: a ca::io::writer (40 bytes), a ca::fs::descriptor (4) and the allocator (0 for a
     stateless one, 8 for arena_allocator). Writes and formatting go through the inner writer:
     same in-place window, same costs, no extra indirection once inlined.
   - The buffer is one allocation from A, 64-byte aligned (a cache line), made once by the
     factory; resize() allocates the new block before releasing the old, so a failed resize
     changes nothing.
   - Moving copies the pointers: the buffer does not move. */
#ifndef ca_resources_io_buffered_writer_hpp
#define ca_resources_io_buffered_writer_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "6-resources/fs/descriptor.hpp"
#include "6-resources/io/buffering.hpp"
#include "6-resources/io/writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- BUFFERED WRITER ------------------------------------------------

		/* @DOC ca::io::writer whose buffer comes from allocator A, sized by the caller and
		   resizable; over a borrowed descriptor (an int or a ca::fs::descriptor that outlives it)
		   or an adopted one (a ca::fs::descriptor moved in), closed by cleanup(). Build with
		   over/adopt, or make_in<buffered_writer<>>(arena, fd, mode, capacity). A writer itself:
		   print and format_to write into it. Cleanup is cleanup() (flush, free, close: first
		   error); the destructor is a failsafe doing the same silently. */
		template <ca::is_allocator A = ca::heap>
		class buffered_writer final {


			private:

				// -- private types ------------------------------------------

				using self = ca::io::buffered_writer<A>;


			public:

				// -- public types -------------------------------------------

				using allocator_type = A;

				template <ca::is_allocator B>
				using rebind = ca::io::buffered_writer<B>;


				// -- public constants ---------------------------------------

				/* @DOC Alignment of the buffer: one cache line. */
				static constexpr ca::usz ALIGNMENT = 64U;


			private:

				// -- private members ----------------------------------------

				ca::fs::descriptor _owned;
				ca::io::writer _writer;
				[[no_unique_address]] A _allocator;


			public:

				// -- public lifecycle ---------------------------------------

				buffered_writer(const self&) = delete;

				buffered_writer(self&& other) noexcept
				: _owned{ca::move(other._owned)}, _writer{ca::move(other._writer)}, _allocator{other._allocator} {
				}

				// Failsafe for a skipped cleanup(): flushes, frees and closes, ignoring errors.
				// ~buffered_writer(void) noexcept {
				//
				// 	static_cast<void>(_writer.flush());
				// 	self::_free(_allocator, _writer.buffer());
				// 	static_cast<void>(_writer.set_buffer(ca::span<char>{}));
				// }

				/* @DOC Over fd, borrowed, with a capacity-byte buffer from allocator. */
				[[nodiscard]] static auto over(const A& allocator, const int fd, const ca::io::buffering mode, const ca::usz capacity) noexcept
				-> ca::result<self> {
					return self::_build(allocator, ca::fs::descriptor::adopt(-1), fd, mode, capacity);
				}

				[[nodiscard]] static auto over(const A& allocator, const ca::fs::descriptor& file, const ca::io::buffering mode, const ca::usz capacity) noexcept
				-> ca::result<self> {
					return self::_build(allocator, ca::fs::descriptor::adopt(-1), file.native(), mode, capacity);
				}

				[[nodiscard]] static auto over(const int fd, const ca::io::buffering mode, const ca::usz capacity) noexcept
				-> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::over(A{}, fd, mode, capacity);
				}

				[[nodiscard]] static auto over(const ca::fs::descriptor& file, const ca::io::buffering mode, const ca::usz capacity) noexcept
				-> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::over(A{}, file, mode, capacity);
				}

				/* @DOC Takes file, closed by cleanup(); on failure file is closed too. */
				[[nodiscard]] static auto adopt(const A& allocator, ca::fs::descriptor&& file, const ca::io::buffering mode, const ca::usz capacity) noexcept
				-> ca::result<self> {

					const int fd = file.native();
					return self::_build(allocator, ca::move(file), fd, mode, capacity);
				}

				[[nodiscard]] static auto adopt(ca::fs::descriptor&& file, const ca::io::buffering mode, const ca::usz capacity) noexcept
				-> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::adopt(A{}, ca::move(file), mode, capacity);
				}

				/* @DOC over or adopt, for ca::make_in. */
				template <typename F>
				[[nodiscard]] static auto make_in(const A& allocator, F&& file, const ca::io::buffering mode, const ca::usz capacity) noexcept
				-> ca::result<self> {

					if constexpr (requires { self::adopt(allocator, static_cast<F&&>(file), mode, capacity); })
						return self::adopt(allocator, static_cast<F&&>(file), mode, capacity);
					else
						return self::over(allocator, file, mode, capacity);
				}


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				auto operator=(self&&) -> self& = delete;


				// -- accessors ----------------------------------------------

				/* @DOC The inner writer, for code taking a ca::io::writer&. */
				[[nodiscard]] constexpr auto writer(void) noexcept -> ca::io::writer& {
					return _writer;
				}

				[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
					return _allocator;
				}

				[[nodiscard]] constexpr auto native(void) const noexcept -> int {
					return _writer.native();
				}

				/* @DOC Whether cleanup() closes the descriptor. */
				[[nodiscard]] constexpr auto owns_descriptor(void) const noexcept -> bool {
					return _owned.is_open();
				}

				[[nodiscard]] constexpr auto buffering(void) const noexcept -> ca::io::buffering {
					return _writer.buffering();
				}

				[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
					return _writer.capacity();
				}

				[[nodiscard]] constexpr auto pending(void) const noexcept -> ca::usz {
					return _writer.pending();
				}

				[[nodiscard]] constexpr auto error(void) const noexcept -> ca::error {
					return _writer.error();
				}


				// -- methods ------------------------------------------------

				/* @DOC See ca::io::writer::write. */
				[[nodiscard]] auto write(const ca::span<const ca::byte> data) noexcept -> ca::result<void> {
					return _writer.write(data);
				}

				[[nodiscard]] auto flush(void) noexcept -> ca::result<void> {
					return _writer.flush();
				}

				constexpr auto clear_error(void) noexcept -> void {
					_writer.clear_error();
				}

				constexpr auto discard(void) noexcept -> void {
					_writer.discard();
				}

				[[nodiscard]] auto set_buffering(const ca::io::buffering mode) noexcept -> ca::result<void> {
					return _writer.set_buffering(mode);
				}

				/* @DOC A capacity-byte buffer instead, pending bytes written out first (that write-out's
				   error returned, the switch done anyway; see ca::io::writer::set_buffer). Allocation
				   failure: nothing changes. 0 frees it: unbuffered. */
				[[nodiscard]] auto resize(const ca::usz capacity) noexcept -> ca::result<void> {

					if (capacity == _writer.capacity())
						return _writer.write_pending();
					ca::span<char> fresh{};
					if (capacity != 0U) {
						auto block = _allocator.allocate(capacity, ALIGNMENT);
						if (!block) [[unlikely]]
							return block.propagate();
						fresh = ca::span<char>{static_cast<char*>(block->data()), capacity};
					}
					const ca::span<char> old = _writer.buffer();
					auto flushed = _writer.set_buffer(fresh);
					self::_free(_allocator, old);
					return flushed;
				}

				/* @DOC Flushes, frees the buffer, closes an adopted descriptor; the first error. The
				   writer is closed afterwards whatever happened. */
				[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

					auto flushed = _writer.flush();
					self::_free(_allocator, _writer.buffer());
					ca::destroy_at(&_writer);
					ca::construct_at(&_writer);
					auto closed = _owned.cleanup();
					if (!flushed) [[unlikely]]
						return flushed;
					return closed;
				}

				[[nodiscard]] auto window(void) const noexcept -> ca::span<char> {
					return _writer.window();
				}

				[[nodiscard]] auto drain(char* const at) noexcept -> ca::result<ca::span<char>> {
					return _writer.drain(at);
				}

				[[nodiscard]] auto settle(char* const at) noexcept -> ca::result<void> {
					return _writer.settle(at);
				}


			private:

				// -- private methods ----------------------------------------

				buffered_writer(ca::fs::descriptor&& owned, const int fd, const ca::io::buffering mode, const ca::span<char> buffer, const A& allocator) noexcept
				: _owned{ca::move(owned)}, _writer{fd, mode, buffer}, _allocator{allocator} {
				}

				[[nodiscard]] static auto _build(const A& allocator, ca::fs::descriptor&& owned, const int fd, const ca::io::buffering mode, const ca::usz capacity) noexcept
				-> ca::result<self> {

					ca::span<char> buffer{};
					if (capacity != 0U) {
						auto block = allocator.allocate(capacity, ALIGNMENT);
						if (!block) [[unlikely]] {
							static_cast<void>(owned.cleanup());
							return block.propagate();
						}
						buffer = ca::span<char>{static_cast<char*>(block->data()), capacity};
					}
					return self{ca::move(owned), fd, mode, buffer, allocator};
				}

				static auto _free(const A& allocator, const ca::span<char> buffer) noexcept -> void {
					if (buffer.data() != nullptr)
						allocator.deallocate(ca::allocation{buffer.data(), buffer.size()}, ALIGNMENT);
				}

		}; // class buffered_writer

	} // namespace io

} // namespace ca

#endif // ca_resources_io_buffered_writer_hpp
