#include "1-primitives/integers/u16.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/io/get_terminal_size.hpp"

#include <sys/ioctl.h>


[[nodiscard]] auto ca::sys::get_terminal_size(const int fd) noexcept -> ca::result<ca::sys::terminal_size> {

	winsize size{};
	if (::ioctl(fd, TIOCGWINSZ, &size) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::sys::terminal_size{static_cast<ca::u16>(size.ws_row), static_cast<ca::u16>(size.ws_col)};
}
