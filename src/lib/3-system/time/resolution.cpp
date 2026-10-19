#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/syscall/syscalls.hpp"
#include "3-system/time/resolution.hpp"

#include <time.h>


[[nodiscard]] auto ca::sys::resolution(const ca::sys::clock source) noexcept -> ca::i64 {

	const clockid_t id = source == ca::sys::clock::realtime ? CLOCK_REALTIME
		: source == ca::sys::clock::monotonic ? CLOCK_MONOTONIC
		: source == ca::sys::clock::process_cpu ? CLOCK_PROCESS_CPUTIME_ID : CLOCK_THREAD_CPUTIME_ID;
	timespec step{};
#if ca_platform(linux) && ca_architecture(x86_64)
	static_cast<void>(ca::syscall::clock_getres(id, &step));
#else
	::clock_getres(id, &step);
#endif
	return static_cast<ca::i64>(step.tv_sec) * 1000000000 + static_cast<ca::i64>(step.tv_nsec);
}
