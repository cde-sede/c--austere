/* @DOC Performance of ca::spsc_queue<T, A>.

   - Layout: the handle is a pointer to the shared block, the slots, the index mask and the
     allocator (24 bytes with ca::heap); every thread reads it, none writes it. The block holds
     the consumer's line (its index and the last producer index it saw), the producer's line
     (the mirror), then the slots: no line written by both sides.
   - Each side reads the other's index only when its cached copy says the queue looks full
     (producer) or empty (consumer): a stream of pushes and pops makes one acquire load per
     lap of the other side, not per element. Publishing is one release store, a plain mov on
     x86: no locked instruction, no fence, on either side.
   - Capacity is a power of two: indices are free-running counts masked into the slots.
   - Time: try_push, try_emplace, try_pop O(1); cleanup O(n) for elements left.
   - Space: two 128-byte lines and capacity slots, one allocation from A (128-byte aligned, or
     T's alignment when larger), made by the factory; never grows.
   - Copy: none; the handle moves (the block stays where it is), but not while shared.

   Baseline a std::deque<T> behind a std::mutex, the plain thread-safe FIFO; one thread pushing
   and popping in turn, so the figures are the cost of the operations themselves, without the
   cache-line transfers of two threads (the harness runs on one CPU):
   --BENCH bench/queues.cpp spsc
   BENCH-- */
#ifndef ca_collections_concurrent_spsc_queue_hpp
#define ca_collections_concurrent_spsc_queue_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_object.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "5-collections/concurrent/impl/queue_parts.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SPSC QUEUE ---------------------------------------------------------

	namespace impl {

		/* @DOC The shared indices of an spsc_queue: each side's line holds the index it writes and
		   its cached copy of the other's. */
		struct spsc_indices final {
				using self = ca::impl::spsc_indices;
				alignas(ca::impl::queue_line) ca::atomic<ca::usz> _head;
				ca::usz _tail_seen;
				alignas(ca::impl::queue_line) ca::atomic<ca::usz> _tail;
				ca::usz _head_seen;
		}; // struct spsc_indices

	} // namespace impl


	/* @DOC Bounded lock-free FIFO of T between one producer thread and one consumer thread
	   (Lamport's ring with cached indices): try_push and try_emplace on the producer only,
	   try_pop on the consumer only, none of them ever blocks or allocates. A failed push leaves
	   its argument untouched. Build with make / make_in(allocator, capacity); share the queue by
	   reference, and neither move it nor clean it up while either side may use it. cleanup()
	   cleans up and destroys the elements left, then frees the block. */
	template <typename T, ca::is_allocator A = ca::heap>
	class spsc_queue final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::spsc_queue<T, A>;


		public:

			// -- public types -----------------------------------------------

			using value_type     = T;
			using allocator_type = A;
			using size_type      = ca::usz;

			/* @DOC The same queue type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::spsc_queue<T, B>;


		private:

			// -- private members --------------------------------------------

			ca::impl::spsc_indices* _indices;
			T*                      _slots;
			ca::usz                 _mask;
			[[no_unique_address]] A _allocator;


		public:

			// -- public lifecycle -------------------------------------------

			spsc_queue(const self&) = delete;

			spsc_queue(self&& other) noexcept
			: _indices{ca::exchange(other._indices, nullptr)}, _slots{ca::exchange(other._slots, nullptr)}, _mask{ca::exchange(other._mask, 0U)}, _allocator{other._allocator} {
			}

			/* @DOC A queue of at least capacity elements (> 0), rounded up to a power of two, from
			   allocator. */
			[[nodiscard]] static auto make_in(const A& allocator, const ca::usz capacity) noexcept -> ca::result<self> {

				const auto count = ca::impl::queue_capacity(capacity, 1U);
				if (!count) [[unlikely]]
					return count.propagate();
				const auto layout = ca::impl::queue_layout(sizeof(ca::impl::spsc_indices), *count, sizeof(T), alignof(T));
				if (!layout) [[unlikely]]
					return layout.propagate();
				auto block = allocator.allocate(layout->_size, layout->_alignment);
				if (!block) [[unlikely]]
					return block.propagate();
				ca::byte* const base = static_cast<ca::byte*>(block->data());
				auto* const indices = ca::construct_at(reinterpret_cast<ca::impl::spsc_indices*>(base));
				return self{indices, reinterpret_cast<T*>(base + layout->_slots_at), *count - 1U, allocator};
			}

			[[nodiscard]] static auto make(const ca::usz capacity) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
				return self::make_in(A{}, capacity);
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's queue and allocator; the previous one is released as by cleanup().
			   Traps when T is cleanable and elements remain: cleanup() first. */
			auto operator=(self&& other) noexcept -> self& {

				if (this == &other)
					return *this;
				if constexpr (ca::is_cleanable<T>) {
					if (_indices != nullptr && this->size() != 0U) [[unlikely]]
						ca::impl::assigned_over_uncleaned_elements();
				}
				static_cast<void>(this->cleanup());
				  _indices = ca::exchange(other._indices, nullptr);
				    _slots = ca::exchange(other._slots, nullptr);
				     _mask = ca::exchange(other._mask, 0U);
				_allocator = other._allocator;
				return *this;
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
				return _allocator;
			}

			/* @DOC Elements the queue holds when full; 0 after cleanup. */
			[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
				return _indices == nullptr ? 0U : _mask + 1U;
			}

			/* @DOC Elements pushed and not popped, as of some moment during the call: exact on
			   either side when the other is idle. */
			[[nodiscard]] auto size(void) const noexcept -> ca::usz {

				const ca::usz head = _indices->_head.template load<ca::memory_order::acquire>();
				return _indices->_tail.template load<ca::memory_order::acquire>() - head;
			}


			// -- methods ----------------------------------------------------

			/* @DOC Producer: appends a T made from value, unless the queue is full (errc::no_space),
			   value then untouched. */
			template <typename U>
			requires ca::is_constructible<T, U&&>
			[[nodiscard]] auto try_push(U&& value) noexcept -> ca::result<void> {
				return this->try_emplace(ca::forward<U>(value));
			}

			/* @DOC Producer: appends a T made from args, unless the queue is full (errc::no_space). */
			template <typename... Args>
			requires ca::is_constructible<T, Args&&...>
			[[nodiscard]] auto try_emplace(Args&&... args) noexcept -> ca::result<void> {

				ca::impl::spsc_indices& indices = *_indices;
				const ca::usz tail = indices._tail.template load<ca::memory_order::relaxed>();
				if (tail - indices._head_seen > _mask) {
					indices._head_seen = indices._head.template load<ca::memory_order::acquire>();
					if (tail - indices._head_seen > _mask) [[unlikely]]
						return ca::failure{ca::errc::no_space};
				}
				ca::construct_at(_slots + (tail & _mask), ca::forward<Args>(args)...);
				indices._tail.template store<ca::memory_order::release>(tail + 1U);
				return ca::result<void>{};
			}

			/* @DOC Consumer: the oldest element, moved out, or nothing when the queue is empty. */
			[[nodiscard]] auto try_pop(void) noexcept -> ca::optional<T> {

				ca::impl::spsc_indices& indices = *_indices;
				const ca::usz head = indices._head.template load<ca::memory_order::relaxed>();
				if (head == indices._tail_seen) {
					indices._tail_seen = indices._tail.template load<ca::memory_order::acquire>();
					if (head == indices._tail_seen) [[unlikely]]
						return ca::nullopt;
				}
				T* const slot = _slots + (head & _mask);
				ca::optional<T> value{ca::in_place, ca::move(*slot)};
				ca::destroy_at(slot);
				indices._head.template store<ca::memory_order::release>(head + 1U);
				return value;
			}

			/* @DOC Cleans up and destroys the elements left, then frees the block: capacity 0.
			   Neither side may use the queue during or after it. Returns the first element's cleanup
			   error; everything is released regardless. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

				if (_indices == nullptr)
					return ca::result<void>{};
				ca::error first{};
				const ca::usz tail = _indices->_tail.template load<ca::memory_order::acquire>();
				for (ca::usz at = _indices->_head.template load<ca::memory_order::acquire>(); at != tail; ++at)
					ca::impl::queue_release(_slots + (at & _mask), first);
				const ca::impl::queue_block layout = ca::impl::queue_layout_of(sizeof(ca::impl::spsc_indices), _mask + 1U, sizeof(T), alignof(T));
				_allocator.deallocate(ca::allocation{_indices, layout._size}, layout._alignment);
				_indices = nullptr;
				  _slots = nullptr;
				   _mask = 0U;
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}


		private:

			// -- private methods --------------------------------------------

			spsc_queue(ca::impl::spsc_indices* const indices, T* const slots, const ca::usz mask, const A& allocator) noexcept
			: _indices{indices}, _slots{slots}, _mask{mask}, _allocator{allocator} {
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<T> && !ca::is_const<T> && !ca::is_array<T>,
				"ca::spsc_queue<T>: T must be a non-const, non-array object type");

	}; // class spsc_queue

} // namespace ca

#endif // ca_collections_concurrent_spsc_queue_hpp
