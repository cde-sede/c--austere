#include "3-system/sync/semaphore.hpp"
#include "3-system/thread/futex_wait.hpp"
#include "3-system/thread/futex_wait_until.hpp"
#include "3-system/thread/futex_wake.hpp"

/* A sleeper counts itself before its last look at the permits; release adds permits before
   reading the count. Both sequentially consistent, so release either sees the sleeper or the
   sleeper sees the permits (the kernel compares the word again under its own lock). */

auto ca::semaphore::_acquire_contended(void) noexcept -> void {

	_waiters.fetch_add(1U);
	while (!try_acquire()) {
		if (_permits.load() == 0U)
			ca::sys::futex_wait(_permits, 0U);
	}
	_waiters.fetch_sub<ca::memory_order::relaxed>(1U);
}

[[nodiscard]] auto ca::semaphore::acquire_until(const ca::i64 deadline) noexcept -> ca::result<void> {

	_waiters.fetch_add(1U);
	ca::result<void> waited{};
	bool taken = try_acquire();
	while (!taken && waited) {
		if (_permits.load() == 0U)
			waited = ca::sys::futex_wait_until(_permits, 0U, deadline);
		taken = try_acquire();
	}
	_waiters.fetch_sub<ca::memory_order::relaxed>(1U);
	return taken ? ca::result<void>{} : waited;
}

auto ca::semaphore::release(const ca::u32 n) noexcept -> void {

	_permits.fetch_add(n);
	if (_waiters.load() != 0U)
		ca::sys::futex_wake(_permits, n);
}
