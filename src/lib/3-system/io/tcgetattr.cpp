#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/io/impl/terminal_layout.hpp"
#include "3-system/io/tcgetattr.hpp"
#include "3-system/syscall/syscalls.hpp"

#if ca_platform(linux)
#include <sys/ioctl.h>
#else
#include <termios.h>
#endif


[[nodiscard]] auto ca::sys::tcgetattr(const int fd) noexcept -> ca::result<ca::sys::terminal_settings> {

	ca::sys::terminal_settings settings{};
#if ca_platform(linux) && ca_architecture(x86_64)
	const auto done = ca::syscall::ioctl(fd, TCGETS, &settings);
	if (!done) [[unlikely]]
		return done.propagate();
#elif ca_platform(linux)
	if (::ioctl(fd, TCGETS, &settings) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
#else
	::termios native{};
	if (::tcgetattr(fd, &native) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	settings = ca::sys::impl::terminal_layout::from_native(&native);
#endif
	return settings;
}
