#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/mmap_anonymous_fixed.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/mman.h>


[[nodiscard]] auto ca::sys::mmap_anonymous_fixed(const ca::span<ca::byte> region, const int protection) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto mapped = ca::syscall::mmap(region.data(), region.size(), protection, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	if (!mapped) [[unlikely]]
		return mapped.propagate();
	return ca::result<void>{};
#else
	if (::mmap(region.data(), region.size(), protection, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0) == MAP_FAILED) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
