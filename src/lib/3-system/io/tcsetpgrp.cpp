#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/io/tcsetpgrp.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::tcsetpgrp(const int fd, const ca::i32 group) noexcept -> ca::result<void> {

	const pid_t native = static_cast<pid_t>(group);
#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::ioctl(fd, TIOCSPGRP, &native).transform([](ca::isz) {});
#else
	if (::tcsetpgrp(fd, native) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
