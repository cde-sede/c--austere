#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/io/isatty.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/ioctl.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::isatty(const int fd) noexcept -> bool {

#if ca_platform(linux) && ca_architecture(x86_64)
	alignas(8) char settings[64];
	return static_cast<bool>(ca::syscall::ioctl(fd, TCGETS, settings));
#else
	return ::isatty(fd) == 1;
#endif
}
