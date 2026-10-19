#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u64.hpp"
#include "3-system/machine/processor_count.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::processor_count(void) noexcept -> ca::usz {

#if ca_platform(linux) && ca_architecture(x86_64)
	ca::u64 mask[16] = {};
	const auto filled = ca::syscall::sched_getaffinity(0, sizeof(mask), mask);
	if (!filled) [[unlikely]]
		return 1U;
	ca::usz count = 0U;
	for (ca::usz i = 0U; i < static_cast<ca::usz>(*filled) / sizeof(ca::u64); ++i)
		count += static_cast<ca::usz>(__builtin_popcountll(mask[i]));
	return count < 1U ? 1U : count;
#else
	const long count = ::sysconf(_SC_NPROCESSORS_ONLN);
	return count < 1 ? 1U : static_cast<ca::usz>(count);
#endif
}
