#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/mmap_file.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/mman.h>
#include <sys/types.h>


[[nodiscard]] auto ca::sys::mmap_file(const int fd, const ca::u64 offset, const ca::usz size, const int protection, const ca::sys::sharing sharing) noexcept -> ca::result<ca::span<ca::byte>> {

	const int flags = sharing == ca::sys::sharing::shared ? MAP_SHARED : MAP_PRIVATE;
#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::mmap(nullptr, size, protection, flags, fd, offset).transform([size](const ca::isz address) {
		return ca::span<ca::byte>{reinterpret_cast<ca::byte*>(address), size};
	});
#else
	void* const block = ::mmap(nullptr, size, protection, flags, fd, static_cast<off_t>(offset));
	if (block == MAP_FAILED) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::span<ca::byte>{static_cast<ca::byte*>(block), size};
#endif
}
