#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/fchown.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::fchown(const int fd, const ca::u32 user, const ca::u32 group) noexcept -> ca::result<void> {

	if (::fchown(fd, static_cast<uid_t>(user), static_cast<gid_t>(group)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
