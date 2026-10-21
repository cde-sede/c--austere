#ifndef ca_system_sync_semaphore_hpp
#define ca_system_sync_semaphore_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/time/impl/deadline_after.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SEMAPHORE ----------------------------------------------------------

	/* @DOC A counter of permits: acquire takes one, sleeping while there is none; release gives
	   some back. A count of sleepers keeps release free of syscalls when nobody waits.
	   std::counting_semaphore without the bound. Trivially destructible, neither copyable nor
	   movable. */
	class semaphore final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::semaphore;


			// -- private members --------------------------------------------

			ca::atomic<ca::u32> _permits;
			ca::atomic<ca::u32> _waiters;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr explicit semaphore(const ca::u32 permits) noexcept
			: _permits{permits}, _waiters{0U} {
			}

			semaphore(const self&) = delete;


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;


			// -- methods ----------------------------------------------------

			[[nodiscard]] auto try_acquire(void) noexcept -> bool {

				ca::u32 permits = _permits.load<ca::memory_order::relaxed>();
				while (permits != 0U) {
					if (_permits.compare_exchange_weak<ca::memory_order::acquire>(permits, permits - 1U))
						return true;
				}
				return false;
			}

			auto acquire(void) noexcept -> void {
				if (!try_acquire()) [[unlikely]]
					_acquire_contended();
			}

			/* @DOC acquire, giving up with errc::timed_out at deadline (sys::now(clock::monotonic)
			   nanoseconds). */
			[[nodiscard]] auto acquire_until(ca::i64 deadline) noexcept -> ca::result<void>;

			[[nodiscard]] auto acquire_for(const ca::i64 nanoseconds) noexcept -> ca::result<void> {
				return try_acquire() ? ca::result<void>{} : acquire_until(ca::sys::impl::deadline_after(nanoseconds));
			}

			/* @DOC Gives n permits back, waking up to n sleepers. */
			auto release(ca::u32 n = 1U) noexcept -> void;


		private:

			// -- private methods --------------------------------------------

			auto _acquire_contended(void) noexcept -> void;

	}; // class semaphore

} // namespace ca

#endif // ca_system_sync_semaphore_hpp
