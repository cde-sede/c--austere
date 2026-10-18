#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/mkdir.hpp"

#include <sys/stat.h>


[[nodiscard]] auto ca::sys::mkdir(const char* const path, const ca::u32 mode) noexcept -> ca::result<void> {

	if (::mkdir(path, static_cast<mode_t>(mode)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
