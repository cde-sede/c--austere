#ifndef ca_system_sync_condition_hpp
#define ca_system_sync_condition_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/sync/mutex.hpp"
#include "3-system/time/impl/deadline_after.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CONDITION ----------------------------------------------------------

	/* @DOC Threads sleeping until another signals a change made under a mutex: a sequence number
	   bumped by each notify, the futex word waiters sleep on, and a count of waiters, so a notify
	   with nobody waiting makes no syscall. Waits may return without a notify; the predicate
	   overloads loop until it holds. Constant-initialized, trivially destructible, neither
	   copyable nor movable. */
	class condition final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::condition;


			// -- private members --------------------------------------------

			ca::atomic<ca::u32> _sequence;
			ca::atomic<ca::u32> _waiters;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr condition(void) noexcept
			: _sequence{0U}, _waiters{0U} {
			}

			condition(const self&) = delete;


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;


			// -- methods ----------------------------------------------------

			/* @DOC Unlocks held, sleeps until a notify (or spuriously), locks held again. held must
			   be locked by the caller. */
			auto wait(ca::mutex& held) noexcept -> void;

			template <typename P>
			auto wait(ca::mutex& held, P predicate) noexcept -> void {
				while (!predicate())
					wait(held);
			}

			/* @DOC wait, giving up with errc::timed_out at deadline (sys::now(clock::monotonic)
			   nanoseconds); held is locked again either way. */
			[[nodiscard]] auto wait_until(ca::mutex& held, ca::i64 deadline) noexcept -> ca::result<void>;

			/* @DOC Waits until predicate holds; errc::timed_out when it still does not at deadline. */
			template <typename P>
			[[nodiscard]] auto wait_until(ca::mutex& held, const ca::i64 deadline, P predicate) noexcept -> ca::result<void> {

				while (!predicate()) {
					if (!wait_until(held, deadline))
						return predicate() ? ca::result<void>{} : ca::result<void>{ca::failure{ca::errc::timed_out}};
				}
				return ca::result<void>{};
			}

			[[nodiscard]] auto wait_for(ca::mutex& held, const ca::i64 nanoseconds) noexcept -> ca::result<void> {
				return wait_until(held, ca::sys::impl::deadline_after(nanoseconds));
			}

			template <typename P>
			[[nodiscard]] auto wait_for(ca::mutex& held, const ca::i64 nanoseconds, P predicate) noexcept -> ca::result<void> {
				return wait_until(held, ca::sys::impl::deadline_after(nanoseconds), predicate);
			}

			/* @DOC Wakes one waiter, if any. Holding the mutex is not required. */
			auto notify_one(void) noexcept -> void;

			/* @DOC Wakes every waiter. */
			auto notify_all(void) noexcept -> void;

	}; // class condition

} // namespace ca

#endif // ca_system_sync_condition_hpp
