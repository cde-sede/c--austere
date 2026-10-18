#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/fallocate.hpp"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::fallocate(const int fd, const ca::u64 offset, const ca::u64 size) noexcept -> ca::result<void> {

#if ca_platform(linux)
	int code{EINTR};
	while (code == EINTR)
		code = ::posix_fallocate(fd, static_cast<off_t>(offset), static_cast<off_t>(size));
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	return ca::result<void>{};
#else
	struct ::stat status{};
	if (::fstat(fd, &status) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	const off_t end = static_cast<off_t>(offset + size);
	if (end <= status.st_size)
		return ca::result<void>{};
	fstore_t store{F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, end - status.st_size, 0};
	if (::fcntl(fd, F_PREALLOCATE, &store) == -1) {
		store.fst_flags = F_ALLOCATEALL;
		if (::fcntl(fd, F_PREALLOCATE, &store) == -1) [[unlikely]]
			return ca::failure{ca::sys::impl::libc_error()};
	}
	if (::ftruncate(fd, end) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
