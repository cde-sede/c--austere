#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/rmdir.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::rmdir(const char* const path) noexcept -> ca::result<void> {

	if (::rmdir(path) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
