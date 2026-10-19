#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/open.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <errno.h>
#include <fcntl.h>


[[nodiscard]] auto ca::sys::open(const char* const path, const int flags, const ca::u32 mode) noexcept -> ca::result<int> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::restarting([&] {
		return ca::syscall::openat(AT_FDCWD, path, flags, mode);
	}).transform([](const ca::isz fd) {
		return static_cast<int>(fd);
	});
#else
	int fd{-1};
	do
		fd = ::open(path, flags, static_cast<mode_t>(mode));
	while (fd == -1 && errno == EINTR);
	if (fd == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return fd;
#endif
}
