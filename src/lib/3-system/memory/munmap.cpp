#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/mman.h>


[[nodiscard]] auto ca::sys::munmap(const ca::span<ca::byte> mapping) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto done = ca::syscall::munmap(mapping.data(), mapping.size());
	if (!done) [[unlikely]]
		return done.propagate();
	return ca::result<void>{};
#else
	if (::munmap(mapping.data(), mapping.size()) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
