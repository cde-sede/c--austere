#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/openat.hpp"

#include <errno.h>
#include <fcntl.h>


[[nodiscard]] auto ca::sys::openat(const int dir, const char* const path, const int flags, const ca::u32 mode) noexcept -> ca::result<int> {

	int fd{-1};
	do
		fd = ::openat(dir, path, flags, static_cast<mode_t>(mode));
	while (fd == -1 && errno == EINTR);
	if (fd == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return fd;
}
