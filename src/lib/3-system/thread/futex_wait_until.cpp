#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/syscall/syscalls.hpp"
#include "3-system/thread/futex_wait_until.hpp"
#include "3-system/thread/yield.hpp"
#include "3-system/time/now.hpp"

#if ca_platform(linux)
#	include <linux/futex.h>
#	include <sys/syscall.h>
#	include <time.h>
#endif


[[nodiscard]] auto ca::sys::futex_wait_until(const ca::atomic<ca::u32>& word, const ca::u32 expected, const ca::i64 deadline) noexcept -> ca::result<void> {

#if ca_platform(linux)
	const ca::i64 at = deadline > 0 ? deadline : 0;
	const timespec until{static_cast<time_t>(at / 1000000000), static_cast<long>(at % 1000000000)};
	ca::u32* const address = const_cast<ca::u32*>(reinterpret_cast<const ca::u32*>(&word));
#	if ca_architecture(x86_64)
	const auto waited = ca::syscall::futex(address, FUTEX_WAIT_BITSET_PRIVATE, expected, &until, nullptr, FUTEX_BITSET_MATCH_ANY);
#	else
	const auto waited = ca::syscall::invoke(SYS_futex, address, FUTEX_WAIT_BITSET_PRIVATE, expected, &until, nullptr, FUTEX_BITSET_MATCH_ANY);
#	endif
	if (!waited && waited.error() == ca::errc::timed_out) [[unlikely]]
		return ca::failure{ca::errc::timed_out};
	return ca::result<void>{};
#else
	if (ca::sys::now(ca::sys::clock::monotonic) >= deadline) [[unlikely]]
		return ca::failure{ca::errc::timed_out};
	if (word.load<ca::memory_order::relaxed>() == expected)
		ca::sys::yield();
	return ca::result<void>{};
#endif
}
