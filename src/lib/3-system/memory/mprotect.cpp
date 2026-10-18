#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/mprotect.hpp"

#include <sys/mman.h>


[[nodiscard]] auto ca::sys::mprotect(const ca::span<ca::byte> region, const int protection) noexcept -> ca::result<void> {

	if (::mprotect(region.data(), region.size(), protection) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
