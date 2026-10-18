#include "2-vocabulary/failure.hpp"
#include "3-system/environment/setenv.hpp"
#include "3-system/error/last_error.hpp"

#include <stdlib.h>


[[nodiscard]] auto ca::sys::setenv(const char* const name, const char* const value, const bool overwrite) noexcept -> ca::result<void> {

	if (::setenv(name, value, overwrite ? 1 : 0) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
