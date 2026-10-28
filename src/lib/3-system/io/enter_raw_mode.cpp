#include "3-system/io/enter_raw_mode.hpp"
#include "3-system/io/tcgetattr.hpp"
#include "3-system/io/tcsetattr.hpp"


[[nodiscard]] auto ca::sys::enter_raw_mode(const int fd) noexcept -> ca::result<ca::sys::terminal_settings> {

	const auto previous = ca::sys::tcgetattr(fd);
	if (!previous) [[unlikely]]
		return previous;
	const auto done = ca::sys::tcsetattr(fd, previous->raw(), ca::sys::terminal_change::flush);
	if (!done) [[unlikely]]
		return done.propagate();
	return previous;
}
