#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/mprotect.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/mman.h>


[[nodiscard]] auto ca::sys::mprotect(const ca::span<ca::byte> region, const int protection) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::mprotect(region.data(), region.size(), protection).transform([](ca::isz) {});
#else
	if (::mprotect(region.data(), region.size(), protection) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
