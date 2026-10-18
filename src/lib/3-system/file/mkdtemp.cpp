#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/mkdtemp.hpp"

#include <stdlib.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::mkdtemp(char* const pattern) noexcept -> ca::result<void> {

	if (::mkdtemp(pattern) == nullptr) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
