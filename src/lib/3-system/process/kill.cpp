#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/kill.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <signal.h>
#include <sys/types.h>


[[nodiscard]] auto ca::sys::kill(const ca::i32 pid, const int signal) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::kill(pid, signal).transform([](ca::isz) {});
#else
	if (::kill(static_cast<pid_t>(pid), signal) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
