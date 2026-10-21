#ifndef ca_system_sync_mutex_hpp
#define ca_system_sync_mutex_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MUTEX --------------------------------------------------------------

	/* @DOC Mutual exclusion in one 32-bit word: 0 unlocked, 1 locked, 2 locked with sleepers
	   (Drepper's futex mutex). Locking spins briefly, then sleeps in the kernel; unlocking makes
	   a syscall only when someone sleeps. Not recursive: locking it again from its holder never
	   returns. Constant-initialized, trivially destructible, neither copyable nor movable. */
	class mutex final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::mutex;


			// -- private members --------------------------------------------

			ca::atomic<ca::u32> _state;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr mutex(void) noexcept
			: _state{0U} {
			}

			mutex(const self&) = delete;


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;


			// -- accessors --------------------------------------------------

			/* @DOC Whether some thread holds it; a hint for checks, stale once read. */
			[[nodiscard]] auto is_locked(void) const noexcept -> bool {
				return _state.load<ca::memory_order::relaxed>() != 0U;
			}


			// -- methods ----------------------------------------------------

			auto lock(void) noexcept -> void {

				ca::u32 expected = 0U;
				if (!_state.compare_exchange_strong<ca::memory_order::acquire>(expected, 1U)) [[unlikely]]
					_lock_contended();
			}

			[[nodiscard]] auto try_lock(void) noexcept -> bool {

				ca::u32 expected = 0U;
				return _state.compare_exchange_strong<ca::memory_order::acquire>(expected, 1U);
			}

			/* @DOC lock, giving up with errc::timed_out at deadline (sys::now(clock::monotonic)
			   nanoseconds). */
			[[nodiscard]] auto lock_until(ca::i64 deadline) noexcept -> ca::result<void>;

			/* @DOC lock_until nanoseconds from now. */
			[[nodiscard]] auto lock_for(ca::i64 nanoseconds) noexcept -> ca::result<void>;

			/* @DOC Traps when not locked. */
			auto unlock(void) noexcept -> void {

				const ca::u32 previous = _state.exchange<ca::memory_order::release>(0U);
				if (previous != 1U) [[unlikely]]
					_unlock_contended(previous);
			}


		private:

			// -- private methods --------------------------------------------

			auto _lock_contended(void) noexcept -> void;

			auto _unlock_contended(ca::u32 previous) noexcept -> void;

	}; // class mutex

} // namespace ca

#endif // ca_system_sync_mutex_hpp
