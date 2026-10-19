#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u16.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/io/get_terminal_size.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/ioctl.h>


[[nodiscard]] auto ca::sys::get_terminal_size(const int fd) noexcept -> ca::result<ca::sys::terminal_size> {

	winsize size{};
#if ca_platform(linux) && ca_architecture(x86_64)
	const auto done = ca::syscall::ioctl(fd, TIOCGWINSZ, &size);
	if (!done) [[unlikely]]
		return done.propagate();
#else
	if (::ioctl(fd, TIOCGWINSZ, &size) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
#endif
	return ca::sys::terminal_size{static_cast<ca::u16>(size.ws_row), static_cast<ca::u16>(size.ws_col)};
}
