#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/symlink.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::symlink(const char* const target, const char* const link) noexcept -> ca::result<void> {

	if (::symlink(target, link) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
