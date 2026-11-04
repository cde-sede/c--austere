#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/mmap_file_fixed.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/mman.h>
#include <sys/types.h>


[[nodiscard]] auto ca::sys::mmap_file_fixed(const ca::span<ca::byte> region, const int fd, const ca::u64 offset, const int protection) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto mapped = ca::syscall::mmap(region.data(), region.size(), protection, MAP_PRIVATE | MAP_FIXED, fd, offset);
	if (!mapped) [[unlikely]]
		return mapped.propagate();
	return ca::result<void>{};
#else
	if (::mmap(region.data(), region.size(), protection, MAP_PRIVATE | MAP_FIXED, fd, static_cast<off_t>(offset)) == MAP_FAILED) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
