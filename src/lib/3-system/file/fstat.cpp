#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/fstat.hpp"
#include "3-system/file/impl/status_of.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/stat.h>


[[nodiscard]] auto ca::sys::fstat(const int fd) noexcept -> ca::result<ca::sys::file_status> {

	struct ::stat raw{};
#if ca_platform(linux) && ca_architecture(x86_64)
	const auto done = ca::syscall::fstat(fd, &raw);
	if (!done) [[unlikely]]
		return done.propagate();
#else
	if (::fstat(fd, &raw) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
#endif
	return ca::sys::impl::status_of(&raw);
}
