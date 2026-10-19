#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/syscall/auxiliary.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <fcntl.h>


[[nodiscard]] auto ca::syscall::auxiliary(const ca::u64 type) noexcept -> ca::u64 {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto fd = ca::syscall::restarting([] {
		return ca::syscall::openat(AT_FDCWD, "/proc/self/auxv", O_RDONLY | O_CLOEXEC, 0);
	});
	if (!fd) [[unlikely]]
		return 0U;
	ca::u64 entries[128] = {};
	ca::usz filled = 0U;
	for (;;) {
		const auto done = ca::syscall::restarting([&] {
			return ca::syscall::read(*fd, reinterpret_cast<char*>(entries) + filled, sizeof(entries) - filled);
		});
		if (!done || *done == 0)
			break;
		filled += static_cast<ca::usz>(*done);
	}
	static_cast<void>(ca::syscall::close(*fd));
	for (ca::usz i = 0U; i + 1U < filled / sizeof(ca::u64); i += 2U) {
		if (entries[i] == type)
			return entries[i + 1U];
	}
	return 0U;
#else
	static_cast<void>(type);
	return 0U;
#endif
}
