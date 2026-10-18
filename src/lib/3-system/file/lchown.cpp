#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/lchown.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::lchown(const char* const path, const ca::u32 user, const ca::u32 group) noexcept -> ca::result<void> {

	if (::lchown(path, static_cast<uid_t>(user), static_cast<gid_t>(group)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
