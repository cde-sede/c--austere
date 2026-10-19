#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u32.hpp"
#include "3-system/process/getuid.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/types.h>
#include <unistd.h>


static_assert(sizeof(uid_t) == sizeof(ca::u32));

[[nodiscard]] auto ca::sys::getuid(void) noexcept -> ca::u32 {

#if ca_platform(linux) && ca_architecture(x86_64)
	return static_cast<ca::u32>(ca::syscall::getuid().value_or(0));
#else
	return static_cast<ca::u32>(::getuid());
#endif
}
