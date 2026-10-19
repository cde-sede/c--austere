#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/process/exit_immediately.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <unistd.h>


[[noreturn]] auto ca::sys::exit_immediately(const int status) noexcept -> void {

#if ca_platform(linux) && ca_architecture(x86_64)
	ca::syscall::exit_group(status);
#else
	::_exit(status);
#endif
}
