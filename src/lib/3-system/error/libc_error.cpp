#include "3-system/error/last_error.hpp"

#include <errno.h>


auto ca::sys::impl::libc_error(void) noexcept -> ca::error {

	ca::sys::impl::last_code = errno;
	return ca::error{ca::sys::impl::last_code};
}
