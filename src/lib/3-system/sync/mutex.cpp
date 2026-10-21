#include "1-primitives/atomic/spin_pause.hpp"
#include "1-primitives/debug/trap.hpp"
#include "3-system/sync/mutex.hpp"
#include "3-system/thread/futex_wait.hpp"
#include "3-system/thread/futex_wait_until.hpp"
#include "3-system/thread/futex_wake.hpp"
#include "3-system/time/impl/deadline_after.hpp"


static constexpr int SPINS = 100;

[[noreturn, gnu::cold, gnu::noinline]] static auto mutex_unlocked_while_not_locked(void) noexcept -> void {
	ca::trap("mutex unlocked while not locked");
}

/* Takes it while its holder is about to release it, without sleeping. */
static auto spin(ca::atomic<ca::u32>& state) noexcept -> bool {

	for (int i = 0; i < SPINS; ++i) {
		ca::spin_pause();
		ca::u32 expected = 0U;
		if (state.load<ca::memory_order::relaxed>() == 0U && state.compare_exchange_weak<ca::memory_order::acquire>(expected, 1U))
			return true;
	}
	return false;
}

[[nodiscard]] auto ca::mutex::lock_until(const ca::i64 deadline) noexcept -> ca::result<void> {

	if (try_lock() || spin(_state))
		return ca::result<void>{};
	while (_state.exchange<ca::memory_order::acquire>(2U) != 0U) {
		auto waited = ca::sys::futex_wait_until(_state, 2U, deadline);
		if (!waited) [[unlikely]]
			return waited;
	}
	return ca::result<void>{};
}

[[nodiscard]] auto ca::mutex::lock_for(const ca::i64 nanoseconds) noexcept -> ca::result<void> {
	return try_lock() ? ca::result<void>{} : lock_until(ca::sys::impl::deadline_after(nanoseconds));
}

auto ca::mutex::_lock_contended(void) noexcept -> void {

	if (spin(_state))
		return;
	while (_state.exchange<ca::memory_order::acquire>(2U) != 0U)
		ca::sys::futex_wait(_state, 2U);
}

auto ca::mutex::_unlock_contended(const ca::u32 previous) noexcept -> void {

	if (previous == 0U) [[unlikely]]
		mutex_unlocked_while_not_locked();
	ca::sys::futex_wake(_state, 1U);
}
