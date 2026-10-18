#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/msync.hpp"

#include <sys/mman.h>


[[nodiscard]] auto ca::sys::msync(const ca::span<ca::byte> mapping) noexcept -> ca::result<void> {

	if (::msync(mapping.data(), mapping.size(), MS_SYNC) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
