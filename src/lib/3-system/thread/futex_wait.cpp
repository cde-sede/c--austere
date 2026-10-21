#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/syscall/syscalls.hpp"
#include "3-system/thread/futex_wait.hpp"
#include "3-system/thread/yield.hpp"

#if ca_platform(linux)
#	include <linux/futex.h>
#	include <sys/syscall.h>
#endif


auto ca::sys::futex_wait(const ca::atomic<ca::u32>& word, const ca::u32 expected) noexcept -> void {

#if ca_platform(linux)
	ca::u32* const address = const_cast<ca::u32*>(reinterpret_cast<const ca::u32*>(&word));
#	if ca_architecture(x86_64)
	static_cast<void>(ca::syscall::futex(address, FUTEX_WAIT_PRIVATE, expected, nullptr, nullptr, 0));
#	else
	static_cast<void>(ca::syscall::invoke(SYS_futex, address, FUTEX_WAIT_PRIVATE, expected, nullptr, nullptr, 0));
#	endif
#else
	if (word.load<ca::memory_order::relaxed>() == expected)
		ca::sys::yield();
#endif
}
