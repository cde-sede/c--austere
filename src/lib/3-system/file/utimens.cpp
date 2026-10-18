#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/utimens.hpp"

#include <fcntl.h>
#include <sys/stat.h>


[[nodiscard]] auto ca::sys::utimens(const char* const path) noexcept -> ca::result<void> {

	if (::utimensat(AT_FDCWD, path, nullptr, 0) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
