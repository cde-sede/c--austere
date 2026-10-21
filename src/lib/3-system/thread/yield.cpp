#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/syscall/syscalls.hpp"
#include "3-system/thread/yield.hpp"

#include <sched.h>


auto ca::sys::yield(void) noexcept -> void {

#if ca_platform(linux) && ca_architecture(x86_64)
	static_cast<void>(ca::syscall::sched_yield());
#else
	::sched_yield();
#endif
}
