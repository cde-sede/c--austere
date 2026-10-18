#include "2-vocabulary/failure.hpp"
#include "3-system/environment/unsetenv.hpp"
#include "3-system/error/last_error.hpp"

#include <stdlib.h>


[[nodiscard]] auto ca::sys::unsetenv(const char* const name) noexcept -> ca::result<void> {

	if (::unsetenv(name) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
