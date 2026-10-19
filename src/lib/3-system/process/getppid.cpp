#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/i32.hpp"
#include "3-system/process/getppid.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::getppid(void) noexcept -> ca::i32 {

#if ca_platform(linux) && ca_architecture(x86_64)
	return static_cast<ca::i32>(ca::syscall::getppid().value_or(0));
#else
	return static_cast<ca::i32>(::getppid());
#endif
}
