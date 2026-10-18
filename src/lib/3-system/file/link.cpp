#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/link.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::link(const char* const from, const char* const to) noexcept -> ca::result<void> {

	if (::link(from, to) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
