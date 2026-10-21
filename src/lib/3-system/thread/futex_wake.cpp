#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/syscall/syscalls.hpp"
#include "3-system/thread/futex_wake.hpp"

#if ca_platform(linux)
#	include <limits.h>
#	include <linux/futex.h>
#	include <sys/syscall.h>
#endif


auto ca::sys::futex_wake(const ca::atomic<ca::u32>& word, const ca::u32 count) noexcept -> void {

#if ca_platform(linux)
	ca::u32* const address = const_cast<ca::u32*>(reinterpret_cast<const ca::u32*>(&word));
	const int wake = count > static_cast<ca::u32>(INT_MAX) ? INT_MAX : static_cast<int>(count);
#	if ca_architecture(x86_64)
	static_cast<void>(ca::syscall::futex(address, FUTEX_WAKE_PRIVATE, wake, nullptr, nullptr, 0));
#	else
	static_cast<void>(ca::syscall::invoke(SYS_futex, address, FUTEX_WAKE_PRIVATE, wake, nullptr, nullptr, 0));
#	endif
#else
	static_cast<void>(word);
	static_cast<void>(count);
#endif
}
