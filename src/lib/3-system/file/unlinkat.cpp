#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/unlinkat.hpp"

#include <fcntl.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::unlinkat(const int dir, const char* const path, const int flags) noexcept -> ca::result<void> {

	if (::unlinkat(dir, path, flags) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
