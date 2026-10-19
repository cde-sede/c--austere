#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/getpgid.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::getpgid(const ca::i32 pid) noexcept -> ca::result<ca::i32> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::getpgid(pid).transform([](const ca::isz group) {
		return static_cast<ca::i32>(group);
	});
#else
	const pid_t group = ::getpgid(static_cast<pid_t>(pid));
	if (group == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::i32>(group);
#endif
}
