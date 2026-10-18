#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/chdir.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::chdir(const char* const path) noexcept -> ca::result<void> {

	if (::chdir(path) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
