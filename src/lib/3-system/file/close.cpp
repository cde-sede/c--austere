#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/close.hpp"

#include <errno.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::close(const int fd) noexcept -> ca::result<void> {

	if (::close(fd) == -1 && errno != EINTR) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
