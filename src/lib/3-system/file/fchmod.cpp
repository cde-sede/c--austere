#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/fchmod.hpp"

#include <sys/stat.h>


[[nodiscard]] auto ca::sys::fchmod(const int fd, const ca::u32 mode) noexcept -> ca::result<void> {

	if (::fchmod(fd, static_cast<mode_t>(mode)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
