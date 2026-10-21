#ifndef ca_system_sync_latch_hpp
#define ca_system_sync_latch_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/time/impl/deadline_after.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- LATCH --------------------------------------------------------------

	/* @DOC A countdown threads wait on until it reaches zero, once: std::latch. Counting below
	   zero traps. Trivially destructible, neither copyable nor movable. */
	class latch final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::latch;


			// -- private members --------------------------------------------

			ca::atomic<ca::u32> _count;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr explicit latch(const ca::u32 count) noexcept
			: _count{count} {
			}

			latch(const self&) = delete;


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;


			// -- accessors --------------------------------------------------

			/* @DOC Whether it reached zero; never sleeps. */
			[[nodiscard]] auto try_wait(void) const noexcept -> bool {
				return _count.load<ca::memory_order::acquire>() == 0U;
			}


			// -- methods ----------------------------------------------------

			/* @DOC Counts n down; the arrival at zero wakes every waiter. */
			auto count_down(ca::u32 n = 1U) noexcept -> void;

			/* @DOC Sleeps until zero. */
			auto wait(void) const noexcept -> void;

			/* @DOC wait, giving up with errc::timed_out at deadline (sys::now(clock::monotonic)
			   nanoseconds). */
			[[nodiscard]] auto wait_until(ca::i64 deadline) const noexcept -> ca::result<void>;

			[[nodiscard]] auto wait_for(const ca::i64 nanoseconds) const noexcept -> ca::result<void> {
				return wait_until(ca::sys::impl::deadline_after(nanoseconds));
			}

			/* @DOC count_down(n), then wait. */
			auto arrive_and_wait(const ca::u32 n = 1U) noexcept -> void {

				count_down(n);
				wait();
			}

	}; // class latch

} // namespace ca

#endif // ca_system_sync_latch_hpp
