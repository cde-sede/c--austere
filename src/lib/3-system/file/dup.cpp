#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/dup.hpp"

#include <fcntl.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::dup(const int fd, const bool cloexec) noexcept -> ca::result<int> {

	const int copy = cloexec ? ::fcntl(fd, F_DUPFD_CLOEXEC, 0) : ::dup(fd);
	if (copy == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return copy;
}
