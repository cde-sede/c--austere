#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/fdatasync.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <errno.h>
#include <unistd.h>


#if !(ca_platform(linux) && ca_architecture(x86_64))

[[nodiscard]] static auto sync_data(const int fd) noexcept -> int {

#if ca_platform(linux)
	return ::fdatasync(fd);
#else
	return ::fsync(fd);
#endif
}

#endif

[[nodiscard]] auto ca::sys::fdatasync(const int fd) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::restarting([fd] {
		return ca::syscall::fdatasync(fd);
	}).transform([](ca::isz) {});
#else
	int done{-1};
	do
		done = sync_data(fd);
	while (done == -1 && errno == EINTR);
	if (done == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
