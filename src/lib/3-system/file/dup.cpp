#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/dup.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <fcntl.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::dup(const int fd, const bool cloexec) noexcept -> ca::result<int> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return (cloexec ? ca::syscall::fcntl(fd, F_DUPFD_CLOEXEC, 0) : ca::syscall::dup(fd)).transform([](const ca::isz copy) {
		return static_cast<int>(copy);
	});
#else
	const int copy = cloexec ? ::fcntl(fd, F_DUPFD_CLOEXEC, 0) : ::dup(fd);
	if (copy == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return copy;
#endif
}
