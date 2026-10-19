#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/close.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <errno.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::close(const int fd) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto done = ca::syscall::close(fd);
	if (!done && done.error() != ca::errc::interrupted) [[unlikely]]
		return done.propagate();
	return ca::result<void>{};
#else
	if (::close(fd) == -1 && errno != EINTR) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
