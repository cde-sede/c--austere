#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/mkstemp.hpp"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::mkstemp(char* const pattern, const int flags) noexcept -> ca::result<int> {

	int fd{-1};
	do
		fd = ::mkostemp(pattern, flags);
	while (fd == -1 && errno == EINTR);
	if (fd == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return fd;
}
