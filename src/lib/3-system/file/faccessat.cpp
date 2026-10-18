#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/faccessat.hpp"

#include <fcntl.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::faccessat(const int dir, const char* const path, const int checks, const int flags) noexcept -> ca::result<void> {

	if (::faccessat(dir, path, checks, flags) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
