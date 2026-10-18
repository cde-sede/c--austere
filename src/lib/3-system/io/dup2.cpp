#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/io/dup2.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::dup2(const int fd, const int target) noexcept -> ca::result<void> {

	if (::dup2(fd, target) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
