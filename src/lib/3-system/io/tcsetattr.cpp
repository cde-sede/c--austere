#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/io/impl/terminal_layout.hpp"
#include "3-system/io/tcsetattr.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <errno.h>

#if ca_platform(linux)
#include <sys/ioctl.h>
#else
#include <termios.h>
#endif


[[nodiscard]] auto ca::sys::tcsetattr(const int fd, const ca::sys::terminal_settings& settings, const ca::sys::terminal_change when) noexcept -> ca::result<void> {

#if ca_platform(linux)
	const unsigned long request = when == ca::sys::terminal_change::now ? TCSETS
		: when == ca::sys::terminal_change::drain ? TCSETSW : TCSETSF;
#endif
#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::restarting([&] {
		return ca::syscall::ioctl(fd, request, &settings);
	}).transform([](ca::isz) {});
#elif ca_platform(linux)
	int done{-1};
	do
		done = ::ioctl(fd, request, &settings);
	while (done == -1 && errno == EINTR);
	if (done == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#else
	const int action = when == ca::sys::terminal_change::now ? TCSANOW
		: when == ca::sys::terminal_change::drain ? TCSADRAIN : TCSAFLUSH;
	::termios native{};
	ca::sys::impl::terminal_layout::to_native(settings, &native);
	int done{-1};
	do
		done = ::tcsetattr(fd, action, &native);
	while (done == -1 && errno == EINTR);
	if (done == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
