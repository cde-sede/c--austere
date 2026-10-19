#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/i32.hpp"
#include "3-system/process/getpid.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/types.h>
#include <unistd.h>


static_assert(sizeof(pid_t) == sizeof(ca::i32));

[[nodiscard]] auto ca::sys::getpid(void) noexcept -> ca::i32 {

#if ca_platform(linux) && ca_architecture(x86_64)
	return static_cast<ca::i32>(ca::syscall::getpid().value_or(0));
#else
	return static_cast<ca::i32>(::getpid());
#endif
}
