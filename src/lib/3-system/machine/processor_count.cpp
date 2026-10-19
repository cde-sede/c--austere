#include "1-primitives/bit/popcount.hpp"
#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u64.hpp"
#include "3-system/machine/processor_count.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <unistd.h>


#if ca_platform(linux) && ca_architecture(x86_64)

/* CPUs in the affinity mask, words of it read into mask; 0 when the kernel's mask is larger
   than words * 64 CPUs (EINVAL) or the call fails. Tried with 1024 CPUs, then 32768. */
static auto count_in(ca::u64* const mask, const ca::usz words) noexcept -> ca::usz {

	const auto filled = ca::syscall::sched_getaffinity(0, words * sizeof(ca::u64), mask);
	if (!filled) [[unlikely]]
		return 0U;
	ca::usz count = 0U;
	for (ca::usz i = 0U; i < static_cast<ca::usz>(*filled) / sizeof(ca::u64); ++i)
		count += static_cast<ca::usz>(ca::popcount(mask[i]));
	return count;
}

#endif

[[nodiscard]] auto ca::sys::processor_count(void) noexcept -> ca::usz {

#if ca_platform(linux) && ca_architecture(x86_64)
	ca::u64 small[16] = {};
	ca::usz count = count_in(small, 16U);
	if (count == 0U) [[unlikely]] {
		ca::u64 large[512] = {};
		count = count_in(large, 512U);
	}
	return count < 1U ? 1U : count;
#else
	const long count = ::sysconf(_SC_NPROCESSORS_ONLN);
	return count < 1 ? 1U : static_cast<ca::usz>(count);
#endif
}
