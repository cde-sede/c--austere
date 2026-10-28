#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/io/tcgetpgrp.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::tcgetpgrp(const int fd) noexcept -> ca::result<ca::i32> {

#if ca_platform(linux) && ca_architecture(x86_64)
	pid_t group{0};
	const auto done = ca::syscall::ioctl(fd, TIOCGPGRP, &group);
	if (!done) [[unlikely]]
		return done.propagate();
	return static_cast<ca::i32>(group);
#else
	const pid_t group = ::tcgetpgrp(fd);
	if (group == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::i32>(group);
#endif
}
