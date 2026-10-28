/* @DOC Performance of ca::io::spsc_ring_buffer<A>.

   - Layout: the handle is a pointer to the shared block, the bytes, the capacity and the
     allocator (24 bytes with ca::heap). The block holds the consumer's line (its count and the
     last producer count it saw), the producer's line (its count, the last consumer count it
     saw, the message in progress, the sticky error), then the bytes, 64-byte aligned.
   - Each side reads the other's count only when its cached copy is not enough for the call:
     a read wanting more than it knows is there, a write or window wanting more room. Publishing
     is one release store, a plain mov on x86: no locked instruction, no fence, on either side.
   - read() and write() are one bounds check and at most two memcpy, as in ring_buffer.
     fill()/consume() and window()/drain()/settle() lend the ring itself, so parsing and
     formatting work in place across threads, without a staging copy. A message becomes visible
     to the consumer whole, at settle or at the end of write, never in part.
   - Unlike ring_buffer, an empty ring is not rewound to its start (that would write the
     producer's count from the consumer's side): a window or fill can stop at the end of the
     ring and continue past the wrap.
   - Time: read, write, fill, consume, window, settle O(1) besides the bytes copied.
   - Space: two 128-byte lines and capacity bytes, one allocation from A made by the factory;
     never grows.
   - Copy: none; the handle moves (the block stays where it is), but not while shared.

   Baseline a std::deque<char> behind a std::mutex; ca is a 4 KiB ring; one thread writing and
   reading in turn, so the figures are the cost of the operations themselves, without the
   cache-line transfers of two threads (the harness runs on one CPU):
   --BENCH bench/queues.cpp bytes
   BENCH-- */
#ifndef ca_collections_io_spsc_ring_buffer_hpp
#define ca_collections_io_spsc_ring_buffer_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
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
#include "5-collections/concurrent/impl/queue_parts.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- SPSC RING BUFFER -----------------------------------------------

		namespace impl {

			/* @DOC The shared part of an spsc_ring_buffer: the consumer's line, then the
			   producer's, each with the count it writes and its cached copy of the other's. */
			struct spsc_ring_state final {
					using self = ca::io::impl::spsc_ring_state;
					alignas(ca::impl::queue_line) ca::atomic<ca::usz> _head;
					ca::usz _tail_seen;
					alignas(ca::impl::queue_line) ca::atomic<ca::usz> _tail;
					ca::usz _head_seen;
					ca::usz _pending;
					ca::error _error;
			}; // struct spsc_ring_state

		} // namespace impl


		/* @DOC ring_buffer between two threads: one producer writes (write, window, drain, settle,
		   flush, error, clear_error), one consumer reads (read, read_full, fill, consume); each
		   side calls only its own methods, size() and space() are for both. Lock-free, never
		   blocks: read returns 0 when the ring is empty, a write that does not fit is dropped
		   whole with errc::no_space, also kept sticky on the producer's side until flush() or
		   clear_error(). The consumer sees only whole messages. Build with make /
		   make_in(allocator, capacity); share it by reference, and neither move it nor clean it
		   up while either side may use it. */
		template <ca::is_allocator A = ca::heap>
		class spsc_ring_buffer final {


			private:

				// -- private types ------------------------------------------

				using self  = ca::io::spsc_ring_buffer<A>;
				using state = ca::io::impl::spsc_ring_state;


			public:

				// -- public types -------------------------------------------

				using allocator_type = A;

				template <ca::is_allocator B>
				using rebind = ca::io::spsc_ring_buffer<B>;


				// -- public constants ---------------------------------------

				/* @DOC Alignment of the bytes: one cache line. */
				static constexpr ca::usz ALIGNMENT = 64U;


			private:

				// -- private members ----------------------------------------

				state*                  _state;
				char*                   _data;
				ca::usz                 _capacity;
				[[no_unique_address]] A _allocator;


			public:

				// -- public lifecycle ---------------------------------------

				spsc_ring_buffer(const self&) = delete;

				spsc_ring_buffer(self&& other) noexcept
				: _state{ca::exchange(other._state, nullptr)}, _data{ca::exchange(other._data, nullptr)}, _capacity{ca::exchange(other._capacity, 0U)}, _allocator{other._allocator} {
				}

				/* @DOC A ring of at least capacity bytes (> 0), rounded up to a power of two, from
				   allocator. */
				[[nodiscard]] static auto make_in(const A& allocator, const ca::usz capacity) noexcept -> ca::result<self> {

					const auto count = ca::impl::queue_capacity(capacity, 1U);
					if (!count) [[unlikely]]
						return count.propagate();
					const auto layout = ca::impl::queue_layout(sizeof(state), *count, 1U, ALIGNMENT);
					if (!layout) [[unlikely]]
						return layout.propagate();
					auto block = allocator.allocate(layout->_size, layout->_alignment);
					if (!block) [[unlikely]]
						return block.propagate();
					char* const base = static_cast<char*>(block->data());
					return self{ca::construct_at(reinterpret_cast<state*>(base)), base + layout->_slots_at, *count, allocator};
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
					static_cast<void>(this->cleanup());
					    _state = ca::exchange(other._state, nullptr);
					     _data = ca::exchange(other._data, nullptr);
					 _capacity = ca::exchange(other._capacity, 0U);
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

				/* @DOC Bytes written and not read yet, at some moment during the call. */
				[[nodiscard]] auto size(void) const noexcept -> ca::usz {

					const ca::usz head = _state->_head.template load<ca::memory_order::acquire>();
					return _state->_tail.template load<ca::memory_order::acquire>() - head;
				}

				/* @DOC Bytes a write could take, at some moment during the call. */
				[[nodiscard]] auto space(void) const noexcept -> ca::usz {
					return _capacity - this->size();
				}

				/* @DOC Producer: the sticky error, ok when none; kept until flush() or clear_error(). */
				[[nodiscard]] auto error(void) const noexcept -> ca::error {
					return _state->_error;
				}


				// -- methods ------------------------------------------------

				/* @DOC Consumer: up to buffer.size() bytes, oldest first; 0 when empty. */
				[[nodiscard]] auto read(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {

					state& shared = *_state;
					const ca::usz head = shared._head.template load<ca::memory_order::relaxed>();
					if (shared._tail_seen - head < buffer.size())
						shared._tail_seen = shared._tail.template load<ca::memory_order::acquire>();
					const ca::usz count = ca::min(buffer.size(), shared._tail_seen - head);
					const ca::usz at = head & (_capacity - 1U);
					const ca::usz first = ca::min(count, _capacity - at);
					ca::copy_bytes(buffer.data(), _data + at, first);
					ca::copy_bytes(buffer.data() + first, _data, count - first);
					shared._head.template store<ca::memory_order::release>(head + count);
					return count;
				}

				/* @DOC Consumer: read; everything published is available at once. */
				[[nodiscard]] auto read_full(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {
					return this->read(buffer);
				}

				/* @DOC Consumer: the oldest unread bytes up to the end of the ring, in place; empty when
				   the ring is. The rest, past the wrap, comes after consume. Valid until consume. */
				[[nodiscard]] auto fill(void) noexcept -> ca::result<ca::sv::str> {

					state& shared = *_state;
					const ca::usz head = shared._head.template load<ca::memory_order::relaxed>();
					const ca::usz at = head & (_capacity - 1U);
					if (shared._tail_seen - head < _capacity - at)
						shared._tail_seen = shared._tail.template load<ca::memory_order::acquire>();
					return ca::sv::str{_data + at, ca::min(shared._tail_seen - head, _capacity - at)};
				}

				/* @DOC Consumer: marks count unread bytes (at most those fill or read last saw) as
				   read. */
				auto consume(const ca::usz count) noexcept -> void {

					state& shared = *_state;
					const ca::usz head = shared._head.template load<ca::memory_order::relaxed>();
					shared._head.template store<ca::memory_order::release>(head + ca::min(count, shared._tail_seen - head));
				}

				/* @DOC Producer: appends bytes when they fit, else drops them (errc::no_space). */
				[[nodiscard]] auto write(const ca::span<const ca::byte> bytes) noexcept -> ca::result<void> {

					state& shared = *_state;
					const ca::usz size = bytes.size();
					const ca::usz tail = shared._tail.template load<ca::memory_order::relaxed>();
					if (size > _capacity - (tail - shared._head_seen))
						shared._head_seen = shared._head.template load<ca::memory_order::acquire>();
					if (!shared._error.ok() || size > _capacity - (tail - shared._head_seen)) [[unlikely]]
						return this->_fail(ca::errc::no_space);
					const ca::usz at = tail & (_capacity - 1U);
					const ca::usz first = ca::min(size, _capacity - at);
					ca::copy_bytes(_data + at, bytes.data(), first);
					ca::copy_bytes(_data, bytes.data() + first, size - first);
					shared._tail.template store<ca::memory_order::release>(tail + size);
					return ca::result<void>{};
				}

				/* @DOC Producer: forgets the sticky error, once handled: writes are accepted again. */
				auto clear_error(void) noexcept -> void {
					_state->_error = ca::error{};
				}

				/* @DOC Producer: nothing to write out: returns the sticky error and clears it. */
				[[nodiscard]] auto flush(void) noexcept -> ca::result<void> {

					const ca::error error = ca::exchange(_state->_error, ca::error{});
					if (!error.ok())
						return ca::failure{error};
					return ca::result<void>{};
				}

				/* @DOC Producer: free space after the last write, up to the end of the ring or the
				   oldest unread byte; empty when full or after an error. See is_windowed_writer. */
				[[nodiscard]] auto window(void) noexcept -> ca::span<char> {
					return this->_window(_state->_error.ok());
				}

				/* @DOC Producer: keeps the bytes up to at for the message and lends the space past the
				   wrap; when there is none the whole message is dropped (errc::no_space). */
				[[nodiscard]] auto drain(char* const at) noexcept -> ca::result<ca::span<char>> {

					state& shared = *_state;
					const ca::usz end = shared._tail.template load<ca::memory_order::relaxed>() + shared._pending;
					shared._pending += static_cast<ca::usz>(at - _data) - (end & (_capacity - 1U));
					const ca::span<char> next = this->_window(true);
					if (next.empty()) [[unlikely]] {
						shared._pending = 0U;
						return this->_fail(ca::errc::no_space);
					}
					return next;
				}

				/* @DOC Producer: ends a message at at: its bytes become readable, all at once. */
				[[nodiscard]] auto settle(char* const at) noexcept -> ca::result<void> {

					state& shared = *_state;
					const ca::usz end = shared._tail.template load<ca::memory_order::relaxed>() + shared._pending;
					shared._tail.template store<ca::memory_order::release>(end + static_cast<ca::usz>(at - _data) - (end & (_capacity - 1U)));
					shared._pending = 0U;
					return ca::result<void>{};
				}

				/* @DOC Frees the ring; afterwards capacity 0. Neither side may use it during or after. */
				[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

					if (_state == nullptr)
						return ca::result<void>{};
					const ca::impl::queue_block layout = ca::impl::queue_layout_of(sizeof(state), _capacity, 1U, ALIGNMENT);
					_allocator.deallocate(ca::allocation{_state, layout._size}, layout._alignment);
					   _state = nullptr;
					    _data = nullptr;
					_capacity = 0U;
					return ca::result<void>{};
				}


			private:

				// -- private methods ----------------------------------------

				spsc_ring_buffer(state* const shared, char* const data, const ca::usz capacity, const A& allocator) noexcept
				: _state{shared}, _data{data}, _capacity{capacity}, _allocator{allocator} {
				}

				[[nodiscard]] auto _window(const bool open) noexcept -> ca::span<char> {

					state& shared = *_state;
					const ca::usz end = shared._tail.template load<ca::memory_order::relaxed>() + shared._pending;
					const ca::usz at = end & (_capacity - 1U);
					if (!open) [[unlikely]]
						return ca::span<char>{_data + at, 0U};
					if (_capacity - (end - shared._head_seen) < _capacity - at)
						shared._head_seen = shared._head.template load<ca::memory_order::acquire>();
					return ca::span<char>{_data + at, ca::min(_capacity - (end - shared._head_seen), _capacity - at)};
				}

				[[nodiscard]] auto _fail(const ca::error error) noexcept -> ca::failure<ca::error> {

					if (_state->_error.ok())
						_state->_error = error;
					return ca::failure{error};
				}

		}; // class spsc_ring_buffer

	} // namespace io

} // namespace ca

#endif // ca_collections_io_spsc_ring_buffer_hpp
