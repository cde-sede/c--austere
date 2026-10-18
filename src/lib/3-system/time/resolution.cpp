#include "3-system/time/resolution.hpp"

#include <time.h>


[[nodiscard]] auto ca::sys::resolution(const ca::sys::clock source) noexcept -> ca::i64 {

	const clockid_t id = source == ca::sys::clock::realtime ? CLOCK_REALTIME
		: source == ca::sys::clock::monotonic ? CLOCK_MONOTONIC
		: source == ca::sys::clock::process_cpu ? CLOCK_PROCESS_CPUTIME_ID : CLOCK_THREAD_CPUTIME_ID;
	timespec step{};
	::clock_getres(id, &step);
	return static_cast<ca::i64>(step.tv_sec) * 1000000000 + static_cast<ca::i64>(step.tv_nsec);
}
