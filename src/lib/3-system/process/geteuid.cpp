#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/process/geteuid.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::geteuid(void) noexcept -> ca::u32 {

#if ca_platform(linux) && ca_architecture(x86_64)
	return static_cast<ca::u32>(ca::syscall::geteuid().value_or(0));
#else
	return static_cast<ca::u32>(::geteuid());
#endif
}
