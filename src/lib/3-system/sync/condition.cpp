#include "1-primitives/limits.hpp"
#include "3-system/sync/condition.hpp"
#include "3-system/thread/futex_wait.hpp"
#include "3-system/thread/futex_wait_until.hpp"
#include "3-system/thread/futex_wake.hpp"

/* A waiter counts itself, then reads the sequence; a notifier bumps the sequence, then reads the
   count. Both sequentially consistent, so at least one sees the other: either the notifier sees
   the waiter and wakes it, or the waiter read the new sequence, which only happens when the
   change it waits for was made before it checked its predicate. */

auto ca::condition::wait(ca::mutex& held) noexcept -> void {

	_waiters.fetch_add(1U);
	const ca::u32 seen = _sequence.load();
	held.unlock();
	ca::sys::futex_wait(_sequence, seen);
	_waiters.fetch_sub<ca::memory_order::relaxed>(1U);
	held.lock();
}

[[nodiscard]] auto ca::condition::wait_until(ca::mutex& held, const ca::i64 deadline) noexcept -> ca::result<void> {

	_waiters.fetch_add(1U);
	const ca::u32 seen = _sequence.load();
	held.unlock();
	auto waited = ca::sys::futex_wait_until(_sequence, seen, deadline);
	_waiters.fetch_sub<ca::memory_order::relaxed>(1U);
	held.lock();
	return waited;
}

auto ca::condition::notify_one(void) noexcept -> void {

	_sequence.fetch_add(1U);
	if (_waiters.load() != 0U)
		ca::sys::futex_wake(_sequence, 1U);
}

auto ca::condition::notify_all(void) noexcept -> void {

	_sequence.fetch_add(1U);
	if (_waiters.load() != 0U)
		ca::sys::futex_wake(_sequence, ca::limits<ca::u32>::max());
}
