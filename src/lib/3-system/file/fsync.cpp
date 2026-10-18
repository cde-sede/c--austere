#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/fsync.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::fsync(const int fd) noexcept -> ca::result<void> {

	if (::fsync(fd) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
