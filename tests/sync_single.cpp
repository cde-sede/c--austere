// expect: exit 0
#include "3-system/error/errc.hpp"
#include "3-system/sync/condition.hpp"
#include "3-system/sync/latch.hpp"
#include "3-system/sync/mutex.hpp"
#include "3-system/sync/once.hpp"
#include "3-system/sync/semaphore.hpp"
#include "3-system/thread/futex_wait.hpp"
#include "3-system/thread/futex_wait_until.hpp"
#include "3-system/thread/futex_wake.hpp"
#include "3-system/time/now.hpp"

static constinit ca::mutex global;

auto main(void) -> int {
	ca::atomic<ca::u32> word{5U};
	ca::sys::futex_wait(word, 4U);
	ca::sys::futex_wake(word, 1U);
	const ca::i64 start = ca::sys::now(ca::sys::clock::monotonic);
	auto expired = ca::sys::futex_wait_until(word, 5U, start + 2000000);
	if (expired || expired.error() != ca::errc::timed_out || ca::sys::now(ca::sys::clock::monotonic) < start + 2000000)
		return 1;
	if (!ca::sys::futex_wait_until(word, 4U, start + 1000000000))
		return 2;

	if (global.is_locked() || !global.try_lock() || global.try_lock() || !global.is_locked())
		return 3;
	auto late = global.lock_for(1000000);
	if (late || late.error() != ca::errc::timed_out)
		return 4;
	global.unlock();
	if (!global.lock_for(1000000))
		return 5;
	global.unlock();
	global.lock();
	global.unlock();

	ca::condition changed;
	changed.notify_one();
	changed.notify_all();
	global.lock();
	auto none = changed.wait_for(global, 1000000, [] { return false; });
	const bool relocked = global.is_locked();
	global.unlock();
	if (none || none.error() != ca::errc::timed_out || !relocked)
		return 6;

	ca::once first;
	int runs = 0;
	auto failed = first.call([&]() -> ca::result<void> { ++runs; return ca::failure{ca::errc::try_again}; });
	if (failed || failed.error() != ca::errc::try_again || first.is_done())
		return 7;
	if (!first.call([&] { ++runs; }) || !first.call([&] { ++runs; }) || runs != 2 || !first.is_done())
		return 8;

	ca::latch countdown{3U};
	countdown.count_down();
	auto pending = countdown.wait_for(1000000);
	if (pending || countdown.try_wait())
		return 9;
	countdown.arrive_and_wait(2U);
	if (!countdown.wait_for(0) || !countdown.try_wait())
		return 10;

	ca::semaphore permits{2U};
	if (!permits.try_acquire() || !permits.acquire_for(1000000))
		return 11;
	auto empty = permits.acquire_for(1000000);
	if (empty || empty.error() != ca::errc::timed_out || permits.try_acquire())
		return 12;
	permits.release(2U);
	permits.acquire();
	return permits.try_acquire() && !permits.try_acquire() ? 0 : 13;
}
