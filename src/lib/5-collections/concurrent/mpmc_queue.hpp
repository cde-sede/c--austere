/* @DOC Performance of ca::mpmc_queue<T, A>.

   - Layout: the handle is a pointer to the block, the cells, the index mask and the allocator
     (24 bytes with ca::heap). The block holds the producers' claim counter and the consumers'
     on separate 128-byte lines, then the cells: a sequence number and room for a T each.
   - Dmitry Vyukov's bounded queue: a push claims a position with one compare-exchange on the
     producers' counter, then publishes through the cell's sequence (one release store); a pop
     mirrors it on the consumers' counter. Producers never touch the consumers' line, nor the
     other way round, and two threads meet on a cell only when the queue is near full or
     empty. No lock: a thread stopped in the middle of an operation delays only the element it
     claimed.
   - Time: try_push, try_emplace, try_pop O(1) besides retries of the compare-exchange under
     contention; cleanup O(n) for elements left.
   - Space: two 128-byte lines and capacity cells (8 bytes plus T, padded to T's alignment), one
     allocation from A made by the factory; never grows.
   - Copy: none; the handle moves (the block stays where it is), but not while shared.

   Baseline a std::deque<T> behind a std::mutex; one thread pushing and popping in turn, the
   cost of the operations without contention (the harness runs on one CPU):
   --BENCH bench/queues.cpp mpmc
   BENCH-- */
#ifndef ca_collections_concurrent_mpmc_queue_hpp
#define ca_collections_concurrent_mpmc_queue_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_object.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/forward.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "5-collections/concurrent/impl/mpmc_core.hpp"
#include "5-collections/concurrent/impl/queue_parts.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MPMC QUEUE ---------------------------------------------------------

	/* @DOC Bounded lock-free FIFO of T shared by any number of producer and consumer threads
	   (Vyukov's array queue): try_push, try_emplace and try_pop from any thread, none of them
	   blocking or allocating. A failed push leaves its argument untouched. Build with make /
	   make_in(allocator, capacity); share the queue by reference, and neither move it nor clean
	   it up while a thread may use it. channel<T, A> adds blocking and closing. */
	template <typename T, ca::is_allocator A = ca::heap>
	class mpmc_queue final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::mpmc_queue<T, A>;
			using cell = ca::impl::mpmc_cell<T>;


		public:

			// -- public types -----------------------------------------------

			using value_type     = T;
			using allocator_type = A;
			using size_type      = ca::usz;

			/* @DOC The same queue type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::mpmc_queue<T, B>;


		private:

			// -- private members --------------------------------------------

			ca::impl::mpmc_ring<T>  _ring;
			[[no_unique_address]] A _allocator;


		public:

			// -- public lifecycle -------------------------------------------

			mpmc_queue(const self&) = delete;

			mpmc_queue(self&& other) noexcept
			: _ring{ca::exchange(other._ring, ca::impl::mpmc_ring<T>{nullptr, nullptr, 0U})}, _allocator{other._allocator} {
			}

			/* @DOC A queue of at least capacity elements (> 0), rounded up to a power of two and to
			   at least 2, from allocator. */
			[[nodiscard]] static auto make_in(const A& allocator, const ca::usz capacity) noexcept -> ca::result<self> {

				const auto count = ca::impl::queue_capacity(capacity, 2U);
				if (!count) [[unlikely]]
					return count.propagate();
				const auto layout = ca::impl::queue_layout(sizeof(ca::impl::mpmc_indices), *count, sizeof(cell), alignof(cell));
				if (!layout) [[unlikely]]
					return layout.propagate();
				auto block = allocator.allocate(layout->_size, layout->_alignment);
				if (!block) [[unlikely]]
					return block.propagate();
				ca::byte* const base = static_cast<ca::byte*>(block->data());
				const ca::impl::mpmc_ring<T> ring{ca::construct_at(reinterpret_cast<ca::impl::mpmc_indices*>(base)), reinterpret_cast<cell*>(base + layout->_slots_at), *count - 1U};
				ca::impl::mpmc_start(ring);
				return self{ring, allocator};
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
					if (_ring._indices != nullptr && ca::impl::mpmc_size(_ring) != 0U) [[unlikely]]
						ca::impl::assigned_over_uncleaned_elements();
				}
				static_cast<void>(this->cleanup());
				     _ring = ca::exchange(other._ring, ca::impl::mpmc_ring<T>{nullptr, nullptr, 0U});
				_allocator = other._allocator;
				return *this;
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
				return _allocator;
			}

			/* @DOC Elements the queue holds when full; 0 after cleanup. */
			[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
				return _ring._indices == nullptr ? 0U : _ring._mask + 1U;
			}

			/* @DOC Pushes claimed minus pops claimed, at some moment during the call: exact when no
			   thread is pushing or popping. */
			[[nodiscard]] auto size(void) const noexcept -> ca::usz {
				return ca::impl::mpmc_size(_ring);
			}


			// -- methods ----------------------------------------------------

			/* @DOC Appends a T made from value, unless the queue is full (errc::no_space), value then
			   untouched. */
			template <typename U>
			requires ca::is_constructible<T, U&&>
			[[nodiscard]] auto try_push(U&& value) noexcept -> ca::result<void> {
				return this->try_emplace(ca::forward<U>(value));
			}

			/* @DOC Appends a T made from args, unless the queue is full (errc::no_space). */
			template <typename... Args>
			requires ca::is_constructible<T, Args&&...>
			[[nodiscard]] auto try_emplace(Args&&... args) noexcept -> ca::result<void> {

				if (!ca::impl::mpmc_try_emplace(_ring, ca::forward<Args>(args)...)) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				return ca::result<void>{};
			}

			/* @DOC The oldest element, moved out, or nothing when the queue is empty. */
			[[nodiscard]] auto try_pop(void) noexcept -> ca::optional<T> {
				return ca::impl::mpmc_try_pop(_ring);
			}

			/* @DOC Cleans up and destroys the elements left, then frees the block: capacity 0. No
			   thread may use the queue during or after it. Returns the first element's cleanup error;
			   everything is released regardless. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

				if (_ring._indices == nullptr)
					return ca::result<void>{};
				ca::error first{};
				ca::impl::mpmc_release(_ring, first);
				const ca::impl::queue_block layout = ca::impl::queue_layout_of(sizeof(ca::impl::mpmc_indices), _ring._mask + 1U, sizeof(cell), alignof(cell));
				_allocator.deallocate(ca::allocation{_ring._indices, layout._size}, layout._alignment);
				_ring = ca::impl::mpmc_ring<T>{nullptr, nullptr, 0U};
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}


		private:

			// -- private methods --------------------------------------------

			mpmc_queue(const ca::impl::mpmc_ring<T> ring, const A& allocator) noexcept
			: _ring{ring}, _allocator{allocator} {
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<T> && !ca::is_const<T> && !ca::is_array<T>,
				"ca::mpmc_queue<T>: T must be a non-const, non-array object type");

	}; // class mpmc_queue

} // namespace ca

#endif // ca_collections_concurrent_mpmc_queue_hpp
