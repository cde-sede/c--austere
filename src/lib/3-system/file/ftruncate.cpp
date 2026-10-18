#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/ftruncate.hpp"

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::ftruncate(const int fd, const ca::u64 size) noexcept -> ca::result<void> {

	int done{-1};
	do
		done = ::ftruncate(fd, static_cast<off_t>(size));
	while (done == -1 && errno == EINTR);
	if (done == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
