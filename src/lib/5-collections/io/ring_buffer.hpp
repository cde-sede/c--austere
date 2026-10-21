/* @DOC In-memory byte stream over a fixed ring of bytes from an allocator.

   Performance:
   - Layout: buffer pointer, capacity, read and write counts, the message in progress, the
     sticky error and the allocator (0 bytes for a stateless one, 8 for arena_allocator);
     48 bytes on 64-bit with ca::heap.
   - Capacity is a power of two: positions are free-running counts masked into the ring, no
     modulo, no branch on wrap besides the split copy.
   - read() and write() are one bounds check and at most two memcpy (one when the bytes do
     not cross the end of the ring). fill()/consume() and window()/drain()/settle() lend the
     ring itself: parsing and formatting work in place, no staging copy.
   - Emptying the ring (read or consume reaching the write count) rewinds both counts to the
     start, so the next window and fill are as long as possible.
   - Time: read, write, fill, consume, window, settle O(1) besides the bytes copied.
   - Space: capacity bytes, one allocation from A made by the factory, 64-byte aligned
     (one cache line); never grows.
   - Copy: none (move only, the moved-from ring is empty). Trivially relocatable in
     practice, not declared so.

   Baseline std::deque<char> as a FIFO; ca is a 4 KiB ring:
   --BENCH bench/ring.cpp
   bench/ring.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | 64 B write + read, 1e6 | 0.11x | 0.082x | |
   | 1-256 B messages, 2 KiB in flight, 1e6 | 0.24x | 0.44x | gcc inlines the bounded copies as rep movsb |
   | format "{} {}\n" into the ring, 1e6 | 0.50x | 0.42x | std: std::format_to through a back_inserter |
   BENCH-- */
#ifndef ca_collections_io_ring_buffer_hpp
#define ca_collections_io_ring_buffer_hpp

#include "1-primitives/bit/bit_ceil.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/limits.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/numeric/min.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- RING BUFFER ----------------------------------------------------

		/* @DOC Stream (ca::io::is_stream) over a ring of bytes from allocator A: what write puts in,
		   read takes out, first in first out. Buffered reader and windowed writer, both in place.
		   Never blocks: read returns 0 when the ring is empty (more may come after a write). A
		   write that does not fit is dropped whole: it fails with errc::no_space, also kept
		   sticky; writes then fail until flush() or clear_error() clears it, so the bytes read
		   are always a prefix of whole messages. Build with make / make_in(allocator, capacity), or
		   ca::make_in<ring_buffer<>>(arena, capacity). Cleanup is cleanup(); the destructor is a
		   failsafe doing the same. Not thread-safe. */
		template <ca::is_allocator A = ca::heap>
		class ring_buffer final {


			private:

				// -- private types ------------------------------------------

				using self = ca::io::ring_buffer<A>;


			public:

				// -- public types -------------------------------------------

				using allocator_type = A;

				template <ca::is_allocator B>
				using rebind = ca::io::ring_buffer<B>;


				// -- public constants ---------------------------------------

				/* @DOC Alignment of the ring: one cache line. */
				static constexpr ca::usz ALIGNMENT = 64U;


			private:

				// -- private members ----------------------------------------

				char* _data;
				ca::usz _capacity;
				ca::usz _head;
				ca::usz _tail;
				ca::usz _pending;
				ca::error _error;
				[[no_unique_address]] A _allocator;


			public:

				// -- public lifecycle ---------------------------------------

				ring_buffer(const self&) = delete;

				ring_buffer(self&& other) noexcept
				: _data{ca::exchange(other._data, nullptr)}, _capacity{ca::exchange(other._capacity, 0U)}, _head{ca::exchange(other._head, 0U)}, _tail{ca::exchange(other._tail, 0U)}, _pending{ca::exchange(other._pending, 0U)}, _error{other._error}, _allocator{other._allocator} {
				}

				// Failsafe for a skipped cleanup(): frees the ring.
				// ~ring_buffer(void) noexcept {
				// 	self::_free(_allocator, _data, _capacity);
				// }

				/* @DOC A ring of at least capacity bytes (> 0), rounded up to a power of two, from
				   allocator. */
				[[nodiscard]] static auto make_in(const A& allocator, const ca::usz capacity) noexcept -> ca::result<self> {

					if (capacity == 0U) [[unlikely]]
						return ca::failure{ca::errc::invalid_argument};
					if (capacity > (ca::limits<ca::usz>::max() >> 1U) + 1U) [[unlikely]]
						return ca::failure{ca::errc::out_of_memory};
					const ca::usz rounded = ca::bit_ceil(capacity);
					auto block = allocator.allocate(rounded, ALIGNMENT);
					if (!block) [[unlikely]]
						return block.propagate();
					return self{static_cast<char*>(block->data()), rounded, allocator};
				}

				[[nodiscard]] static auto make(const ca::usz capacity) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::make_in(A{}, capacity);
				}


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				/* @DOC Takes other's ring and allocator; the previous ring is freed. */
				auto operator=(self&& other) noexcept -> self& {

					if (this == &other)
						return *this;
					self::_free(_allocator, _data, _capacity);
					     _data = ca::exchange(other._data, nullptr);
					 _capacity = ca::exchange(other._capacity, 0U);
					     _head = ca::exchange(other._head, 0U);
					     _tail = ca::exchange(other._tail, 0U);
					  _pending = ca::exchange(other._pending, 0U);
					    _error = other._error;
					_allocator = other._allocator;
					return *this;
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
					return _allocator;
				}

				[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
					return _capacity;
				}

				/* @DOC Bytes written and not read yet. */
				[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
					return _tail - _head;
				}

				/* @DOC Bytes a write can take now. */
				[[nodiscard]] constexpr auto space(void) const noexcept -> ca::usz {
					return _capacity - (_tail - _head);
				}

				[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
					return _tail == _head;
				}

				[[nodiscard]] constexpr auto full(void) const noexcept -> bool {
					return _tail - _head == _capacity;
				}

				/* @DOC The sticky error, ok when none; kept until flush(), clear_error() or clear(). */
				[[nodiscard]] constexpr auto error(void) const noexcept -> ca::error {
					return _error;
				}


				// -- methods ------------------------------------------------

				/* @DOC Up to buffer.size() bytes, oldest first; 0 when empty. */
				[[nodiscard]] auto read(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {

					const ca::usz count = ca::min(buffer.size(), _tail - _head);
					const ca::usz at = _head & (_capacity - 1U);
					const ca::usz first = ca::min(count, _capacity - at);
					ca::copy_bytes(buffer.data(), _data + at, first);
					ca::copy_bytes(buffer.data() + first, _data, count - first);
					this->_advance(count);
					return count;
				}

				/* @DOC read: everything buffered is available at once. */
				[[nodiscard]] auto read_full(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {
					return this->read(buffer);
				}

				/* @DOC The oldest unread bytes up to the end of the ring, in place; empty when the ring
				   is. The rest, past the wrap, comes after consume. Valid until the next write. */
				[[nodiscard]] constexpr auto fill(void) const noexcept -> ca::result<ca::sv::str> {

					const ca::usz at = _head & (_capacity - 1U);
					return ca::sv::str{_data + at, ca::min(_tail - _head, _capacity - at)};
				}

				/* @DOC Marks count unread bytes (at most all) as read. */
				constexpr auto consume(const ca::usz count) noexcept -> void {
					this->_advance(ca::min(count, _tail - _head));
				}

				/* @DOC Appends bytes when they fit, else drops them (errc::no_space). */
				[[nodiscard]] auto write(const ca::span<const ca::byte> bytes) noexcept -> ca::result<void> {

					const ca::usz size = bytes.size();
					if (!_error.ok() || size > _capacity - (_tail - _head)) [[unlikely]]
						return this->_fail(ca::errc::no_space);
					const ca::usz at = _tail & (_capacity - 1U);
					const ca::usz first = ca::min(size, _capacity - at);
					ca::copy_bytes(_data + at, bytes.data(), first);
					ca::copy_bytes(_data, bytes.data() + first, size - first);
					_tail += size;
					return ca::result<void>{};
				}

				/* @DOC Forgets the sticky error, once handled: writes are accepted again. */
				constexpr auto clear_error(void) noexcept -> void {
					_error = ca::error{};
				}

				/* @DOC Nothing to write out: returns the sticky error and clears it. */
				[[nodiscard]] constexpr auto flush(void) noexcept -> ca::result<void> {

					const ca::error error = ca::exchange(_error, ca::error{});
					if (!error.ok())
						return ca::failure{error};
					return ca::result<void>{};
				}

				/* @DOC Drops every unread byte and clears the sticky error. */
				constexpr auto clear(void) noexcept -> void {

					 _head = 0U;
					 _tail = 0U;
					_error = ca::error{};
				}

				/* @DOC Free space after the last write, up to the end of the ring or the oldest unread
				   byte; empty when full or after an error. See is_windowed_writer. */
				[[nodiscard]] constexpr auto window(void) const noexcept -> ca::span<char> {
					return self::_window(_data, _capacity, _head, _tail + _pending, _error.ok());
				}

				/* @DOC Keeps the bytes up to at for the message and lends the space past the wrap; when
				   there is none the whole message is dropped (errc::no_space). */
				[[nodiscard]] constexpr auto drain(char* const at) noexcept -> ca::result<ca::span<char>> {

					_pending += static_cast<ca::usz>(at - _data) - ((_tail + _pending) & (_capacity - 1U));
					const ca::span<char> next = self::_window(_data, _capacity, _head, _tail + _pending, true);
					if (next.empty()) [[unlikely]] {
						_pending = 0U;
						return this->_fail(ca::errc::no_space);
					}
					return next;
				}

				/* @DOC Ends a message at at: its bytes become readable. */
				[[nodiscard]] constexpr auto settle(char* const at) noexcept -> ca::result<void> {

					_tail += _pending + static_cast<ca::usz>(at - _data) - ((_tail + _pending) & (_capacity - 1U));
					_pending = 0U;
					return ca::result<void>{};
				}

				/* @DOC Frees the ring; afterwards capacity 0, every write dropped. */
				[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

					self::_free(_allocator, _data, _capacity);
					    _data = nullptr;
					_capacity = 0U;
					    _head = 0U;
					    _tail = 0U;
					 _pending = 0U;
					return ca::result<void>{};
				}


			private:

				// -- private methods ----------------------------------------

				ring_buffer(char* const data, const ca::usz capacity, const A& allocator) noexcept
				: _data{data}, _capacity{capacity}, _head{0U}, _tail{0U}, _pending{0U}, _error{}, _allocator{allocator} {
				}

				[[nodiscard]] static constexpr auto _window(char* const data, const ca::usz capacity, const ca::usz head, const ca::usz tail, const bool open) noexcept -> ca::span<char> {

					char* const at = data + (tail & (capacity - 1U));
					if (!open) [[unlikely]]
						return ca::span<char>{at, 0U};
					return ca::span<char>{at, ca::min(capacity - (tail - head), capacity - (tail & (capacity - 1U)))};
				}

				constexpr auto _advance(const ca::usz count) noexcept -> void {

					_head += count;
					if (_head == _tail) {
						_head = 0U;
						_tail = 0U;
					}
				}

				[[nodiscard]] constexpr auto _fail(const ca::error error) noexcept -> ca::failure<ca::error> {

					if (_error.ok())
						_error = error;
					return ca::failure{error};
				}

				static auto _free(const A& allocator, char* const data, const ca::usz capacity) noexcept -> void {
					if (data != nullptr)
						allocator.deallocate(ca::allocation{data, capacity}, ALIGNMENT);
				}

		}; // class ring_buffer

	} // namespace io

} // namespace ca

#endif // ca_collections_io_ring_buffer_hpp
