#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/symlinkat.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::symlinkat(const char* const target, const int dir, const char* const link) noexcept -> ca::result<void> {

	if (::symlinkat(target, dir, link) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
