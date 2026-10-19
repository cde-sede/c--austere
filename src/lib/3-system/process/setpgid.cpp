#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/setpgid.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::setpgid(const ca::i32 pid, const ca::i32 group) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::setpgid(pid, group).transform([](ca::isz) {});
#else
	if (::setpgid(static_cast<pid_t>(pid), static_cast<pid_t>(group)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
