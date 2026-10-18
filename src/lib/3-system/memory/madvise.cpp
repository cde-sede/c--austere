#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/madvise.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/mman.h>


static auto native(const ca::sys::advice advice) noexcept -> int {
	switch (advice) {
		case ca::sys::advice::random:
			return MADV_RANDOM;
		case ca::sys::advice::sequential:
			return MADV_SEQUENTIAL;
		case ca::sys::advice::will_need:
			return MADV_WILLNEED;
		case ca::sys::advice::dont_need:
			return MADV_DONTNEED;
		default:
			return MADV_NORMAL;
	}
}

[[nodiscard]] auto ca::sys::madvise(const ca::span<ca::byte> region, const ca::sys::advice advice) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto done = ca::syscall::madvise(region.data(), region.size(), native(advice));
	if (!done) [[unlikely]]
		return done.propagate();
	return ca::result<void>{};
#else
	if (::madvise(region.data(), region.size(), native(advice)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
