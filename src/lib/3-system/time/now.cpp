#include "3-system/time/now.hpp"

#include <time.h>


[[nodiscard]] auto ca::sys::now(const ca::sys::clock source) noexcept -> ca::i64 {

	const clockid_t id = source == ca::sys::clock::realtime ? CLOCK_REALTIME
		: source == ca::sys::clock::monotonic ? CLOCK_MONOTONIC
		: source == ca::sys::clock::process_cpu ? CLOCK_PROCESS_CPUTIME_ID : CLOCK_THREAD_CPUTIME_ID;
	timespec time{};
	::clock_gettime(id, &time);
	return static_cast<ca::i64>(time.tv_sec) * 1000000000 + static_cast<ca::i64>(time.tv_nsec);
}
