/* @DOC Performance of ca::channel<T, A>.

   - Layout: the handle is a pointer to the block, the cells, the index mask and the allocator
     (24 bytes with ca::heap). The block holds an mpmc_queue's two claim counters, then the
     line of the consumers' wait word (an event count bumped when an element arrives, the
     number of consumers asleep, the closed flag), the producers' line (the mirror, bumped when
     room appears), then the cells.
   - The elements move through the lock-free mpmc ring; blocking only adds sleeping. A thread
     that finds the ring empty (or full) counts itself as a sleeper, reads the event count,
     tries once more, and only then sleeps on the count with a futex. The other side, after
     each element it moves, issues one full fence and reads the sleeper count: the futex wake,
     a syscall, happens only when a thread actually sleeps. Without waiting, a push or pop costs
     the mpmc operation plus that fence.
   - No lock anywhere, no allocation after the factory; close() wakes every sleeper.
   - Time: O(1) per operation besides the waiting; cleanup O(n) for elements left.
   - Space: four 128-byte lines and capacity cells, one allocation from A.
   - Copy: none; the handle moves (the block stays where it is), but not while shared.

   Baseline a std::deque<T> behind a std::mutex and two std::condition_variable (the textbook
   bounded buffer); one thread pushing and popping in turn, so nobody ever sleeps and the
   figures are the cost of the operations themselves:
   --BENCH bench/queues.cpp channel
   BENCH-- */
#ifndef ca_collections_concurrent_channel_hpp
#define ca_collections_concurrent_channel_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/atomic_thread_fence.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/limits.hpp"
#include "1-primitives/memory/construct_at.hpp"
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
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/thread/futex_wait.hpp"
#include "3-system/thread/futex_wait_until.hpp"
#include "3-system/thread/futex_wake.hpp"
#include "3-system/time/impl/deadline_after.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "5-collections/concurrent/impl/mpmc_core.hpp"
#include "5-collections/concurrent/impl/queue_parts.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CHANNEL ------------------------------------------------------------

	namespace impl {

		/* @DOC Where one side of a channel sleeps: an event count bumped when the other side makes
		   progress, and how many of this side sleep on it. */
		struct channel_wait final {
				using self = ca::impl::channel_wait;
				alignas(ca::impl::queue_line) ca::atomic<ca::u32> _events;
				ca::atomic<ca::u32> _sleepers;
				ca::atomic<ca::u32> _closed;
		}; // struct channel_wait

		/* @DOC The shared part of a channel: the ring's counters, then the consumers' and the
		   producers' wait lines; _closed is used on the consumers' line only. */
		struct channel_state final {
				using self = ca::impl::channel_state;
				ca::impl::mpmc_indices _indices;
				ca::impl::channel_wait _consumers;
				ca::impl::channel_wait _producers;
		}; // struct channel_state

		/* @DOC Wakes one sleeper of wait, if any, after an element moved: the fence orders that
		   move before the look at the sleepers, which a sleeper makes after counting itself. */
		inline auto channel_signal(ca::impl::channel_wait& wait) noexcept -> void {

			ca::atomic_thread_fence();
			if (wait._sleepers.load<ca::memory_order::relaxed>() != 0U) [[unlikely]] {
				wait._events.fetch_add(1U);
				ca::sys::futex_wake(wait._events, 1U);
			}
		}

		/* @DOC Sleeps until woken: what an untimed wait does, never failing. */
		struct channel_sleep final {
				using self = ca::impl::channel_sleep;
				[[nodiscard]] auto operator()(const ca::atomic<ca::u32>& events, const ca::u32 seen) const noexcept -> ca::result<void> {

					ca::sys::futex_wait(events, seen);
					return ca::result<void>{};
				}
		}; // struct channel_sleep

		/* @DOC Sleeps until woken or the deadline, which fails with errc::timed_out. */
		struct channel_sleep_until final {
				using self = ca::impl::channel_sleep_until;
				ca::i64 _deadline;
				[[nodiscard]] auto operator()(const ca::atomic<ca::u32>& events, const ca::u32 seen) const noexcept -> ca::result<void> {
					return ca::sys::futex_wait_until(events, seen, _deadline);
				}
		}; // struct channel_sleep_until

	} // namespace impl


	/* @DOC Bounded FIFO of T between any number of producer and consumer threads that wait: push
	   sleeps while the channel is full, pop while it is empty. Timed variants give up with
	   errc::timed_out (deadlines on the monotonic clock, as sys::now(clock::monotonic)), try_
	   variants never wait. close() ends it: pushes fail with errc::broken_pipe from then on, pops
	   still drain what is left, then fail the same way; every sleeper wakes. A failed push leaves
	   its argument untouched. Build with make / make_in(allocator, capacity); share the channel
	   by reference, and neither move it nor clean it up while a thread may use it. Close after
	   the last push has returned: one racing with close may succeed after a consumer saw the
	   channel closed and empty. */
	template <typename T, ca::is_allocator A = ca::heap>
	class channel final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::channel<T, A>;
			using cell = ca::impl::mpmc_cell<T>;


		public:

			// -- public types -----------------------------------------------

			using value_type     = T;
			using allocator_type = A;
			using size_type      = ca::usz;

			/* @DOC The same channel type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::channel<T, B>;


		private:

			// -- private members --------------------------------------------

			ca::impl::mpmc_ring<T>  _ring;
			[[no_unique_address]] A _allocator;


		public:

			// -- public lifecycle -------------------------------------------

			channel(const self&) = delete;

			channel(self&& other) noexcept
			: _ring{ca::exchange(other._ring, ca::impl::mpmc_ring<T>{nullptr, nullptr, 0U})}, _allocator{other._allocator} {
			}

			/* @DOC An open channel of at least capacity elements (> 0), rounded up to a power of two
			   and to at least 2, from allocator. */
			[[nodiscard]] static auto make_in(const A& allocator, const ca::usz capacity) noexcept -> ca::result<self> {

				const auto count = ca::impl::queue_capacity(capacity, 2U);
				if (!count) [[unlikely]]
					return count.propagate();
				const auto layout = ca::impl::queue_layout(sizeof(ca::impl::channel_state), *count, sizeof(cell), alignof(cell));
				if (!layout) [[unlikely]]
					return layout.propagate();
				auto block = allocator.allocate(layout->_size, layout->_alignment);
				if (!block) [[unlikely]]
					return block.propagate();
				ca::byte* const base = static_cast<ca::byte*>(block->data());
				auto* const state = ca::construct_at(reinterpret_cast<ca::impl::channel_state*>(base));
				const ca::impl::mpmc_ring<T> ring{&state->_indices, reinterpret_cast<cell*>(base + layout->_slots_at), *count - 1U};
				ca::impl::mpmc_start(ring);
				return self{ring, allocator};
			}

			[[nodiscard]] static auto make(const ca::usz capacity) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
				return self::make_in(A{}, capacity);
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's channel and allocator; the previous one is released as by cleanup().
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

			/* @DOC Elements the channel holds when full; 0 after cleanup. */
			[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
				return _ring._indices == nullptr ? 0U : _ring._mask + 1U;
			}

			/* @DOC Pushes claimed minus pops claimed, at some moment during the call. */
			[[nodiscard]] auto size(void) const noexcept -> ca::usz {
				return ca::impl::mpmc_size(_ring);
			}

			/* @DOC Whether close() ran. */
			[[nodiscard]] auto closed(void) const noexcept -> bool {
				return this->_state()._consumers._closed.load() != 0U;
			}


			// -- methods ----------------------------------------------------

			/* @DOC Appends a T made from value, sleeping while the channel is full; errc::broken_pipe
			   once closed, value then untouched. */
			template <typename U>
			requires ca::is_constructible<T, U&&>
			[[nodiscard]] auto push(U&& value) noexcept -> ca::result<void> {
				return this->_push_waiting(ca::impl::channel_sleep{}, ca::forward<U>(value));
			}

			/* @DOC push, giving up with errc::timed_out at deadline (monotonic nanoseconds). */
			template <typename U>
			requires ca::is_constructible<T, U&&>
			[[nodiscard]] auto push_until(U&& value, const ca::i64 deadline) noexcept -> ca::result<void> {
				return this->_push_waiting(ca::impl::channel_sleep_until{deadline}, ca::forward<U>(value));
			}

			/* @DOC push, giving up with errc::timed_out after nanoseconds. */
			template <typename U>
			requires ca::is_constructible<T, U&&>
			[[nodiscard]] auto push_for(U&& value, const ca::i64 nanoseconds) noexcept -> ca::result<void> {
				return this->_push_waiting(ca::impl::channel_sleep_until{ca::sys::impl::deadline_after(nanoseconds)}, ca::forward<U>(value));
			}

			/* @DOC push without waiting: errc::no_space when full, errc::broken_pipe once closed. */
			template <typename U>
			requires ca::is_constructible<T, U&&>
			[[nodiscard]] auto try_push(U&& value) noexcept -> ca::result<void> {

				if (this->closed()) [[unlikely]]
					return ca::failure{ca::errc::broken_pipe};
				if (!ca::impl::mpmc_try_emplace(_ring, ca::forward<U>(value))) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				ca::impl::channel_signal(this->_state()._consumers);
				return ca::result<void>{};
			}

			/* @DOC The oldest element, sleeping while the channel is empty; errc::broken_pipe once it
			   is closed and empty. */
			[[nodiscard]] auto pop(void) noexcept -> ca::result<T> {
				return this->_pop_waiting(ca::impl::channel_sleep{});
			}

			/* @DOC pop, giving up with errc::timed_out at deadline (monotonic nanoseconds). */
			[[nodiscard]] auto pop_until(const ca::i64 deadline) noexcept -> ca::result<T> {
				return this->_pop_waiting(ca::impl::channel_sleep_until{deadline});
			}

			/* @DOC pop, giving up with errc::timed_out after nanoseconds. */
			[[nodiscard]] auto pop_for(const ca::i64 nanoseconds) noexcept -> ca::result<T> {
				return this->_pop_waiting(ca::impl::channel_sleep_until{ca::sys::impl::deadline_after(nanoseconds)});
			}

			/* @DOC The oldest element without waiting, or nothing when the channel is empty. */
			[[nodiscard]] auto try_pop(void) noexcept -> ca::optional<T> {

				auto value = ca::impl::mpmc_try_pop(_ring);
				if (value)
					ca::impl::channel_signal(this->_state()._producers);
				return value;
			}

			/* @DOC Closes the channel and wakes every sleeper. Closing twice changes nothing. */
			auto close(void) noexcept -> void {

				ca::impl::channel_state& state = this->_state();
				state._consumers._closed.store(1U);
				state._consumers._events.fetch_add(1U);
				ca::sys::futex_wake(state._consumers._events, ca::limits<ca::u32>::max());
				state._producers._events.fetch_add(1U);
				ca::sys::futex_wake(state._producers._events, ca::limits<ca::u32>::max());
			}

			/* @DOC Cleans up and destroys the elements left, then frees the block: capacity 0. No
			   thread may use the channel during or after it. Returns the first element's cleanup
			   error; everything is released regardless. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

				if (_ring._indices == nullptr)
					return ca::result<void>{};
				ca::error first{};
				ca::impl::mpmc_release(_ring, first);
				const ca::impl::queue_block layout = ca::impl::queue_layout_of(sizeof(ca::impl::channel_state), _ring._mask + 1U, sizeof(cell), alignof(cell));
				_allocator.deallocate(ca::allocation{_ring._indices, layout._size}, layout._alignment);
				_ring = ca::impl::mpmc_ring<T>{nullptr, nullptr, 0U};
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}


		private:

			// -- private methods --------------------------------------------

			channel(const ca::impl::mpmc_ring<T> ring, const A& allocator) noexcept
			: _ring{ring}, _allocator{allocator} {
			}

			[[nodiscard]] auto _state(void) const noexcept -> ca::impl::channel_state& {
				return *reinterpret_cast<ca::impl::channel_state*>(_ring._indices);
			}

			template <typename S, typename U>
			[[nodiscard]] auto _push_waiting(const S& sleep, U&& value) noexcept -> ca::result<void> {

				ca::impl::channel_state& state = this->_state();
				for (;;) {
					if (state._consumers._closed.load() != 0U) [[unlikely]]
						return ca::failure{ca::errc::broken_pipe};
					if (ca::impl::mpmc_try_emplace(_ring, ca::forward<U>(value))) [[likely]] {
						ca::impl::channel_signal(state._consumers);
						return ca::result<void>{};
					}
					state._producers._sleepers.fetch_add(1U);
					const ca::u32 seen = state._producers._events.load();
					const bool pushed = ca::impl::mpmc_try_emplace(_ring, ca::forward<U>(value));
					const bool closing = !pushed && state._consumers._closed.load() != 0U;
					const ca::result<void> slept = pushed || closing ? ca::result<void>{} : sleep(state._producers._events, seen);
					state._producers._sleepers.template fetch_sub<ca::memory_order::relaxed>(1U);
					if (pushed) {
						ca::impl::channel_signal(state._consumers);
						return ca::result<void>{};
					}
					if (!slept) {
						if (!ca::impl::mpmc_try_emplace(_ring, ca::forward<U>(value)))
							return slept.propagate();
						ca::impl::channel_signal(state._consumers);
						return ca::result<void>{};
					}
				}
			}

			template <typename S>
			[[nodiscard]] auto _pop_waiting(const S& sleep) noexcept -> ca::result<T> {

				ca::impl::channel_state& state = this->_state();
				for (;;) {
					auto value = ca::impl::mpmc_try_pop(_ring);
					if (value) [[likely]] {
						ca::impl::channel_signal(state._producers);
						return ca::move(*value);
					}
					state._consumers._sleepers.fetch_add(1U);
					const ca::u32 seen = state._consumers._events.load();
					value = ca::impl::mpmc_try_pop(_ring);
					const bool closing = !value && state._consumers._closed.load() != 0U;
					const ca::result<void> slept = value || closing ? ca::result<void>{} : sleep(state._consumers._events, seen);
					state._consumers._sleepers.template fetch_sub<ca::memory_order::relaxed>(1U);
					if (!value && (closing || !slept))
						value = ca::impl::mpmc_try_pop(_ring);
					if (value) {
						ca::impl::channel_signal(state._producers);
						return ca::move(*value);
					}
					if (closing)
						return ca::failure{ca::errc::broken_pipe};
					if (!slept)
						return slept.propagate();
				}
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<T> && !ca::is_const<T> && !ca::is_array<T>,
				"ca::channel<T>: T must be a non-const, non-array object type");

	}; // class channel

} // namespace ca

#endif // ca_collections_concurrent_channel_hpp
