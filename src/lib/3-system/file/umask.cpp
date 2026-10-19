#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/file/umask.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/stat.h>


[[nodiscard]] auto ca::sys::umask(const ca::u32 mask) noexcept -> ca::u32 {

#if ca_platform(linux) && ca_architecture(x86_64)
	return static_cast<ca::u32>(ca::syscall::umask(mask).value_or(0));
#else
	return static_cast<ca::u32>(::umask(static_cast<mode_t>(mask)));
#endif
}
