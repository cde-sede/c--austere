#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/impl/wait_status_of.hpp"
#include "3-system/process/waitpid.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>


[[nodiscard]] auto ca::sys::waitpid(const ca::i32 pid) noexcept -> ca::result<ca::sys::wait_status> {

	int raw{0};
#if ca_platform(linux) && ca_architecture(x86_64)
	const auto reaped = ca::syscall::restarting([pid, &raw] {
		return ca::syscall::wait4(pid, &raw, 0, nullptr);
	});
	if (!reaped) [[unlikely]]
		return reaped.propagate();
	return ca::sys::impl::wait_status_of(raw);
#else
	pid_t reaped{-1};
	do
		reaped = ::waitpid(static_cast<pid_t>(pid), &raw, 0);
	while (reaped == -1 && errno == EINTR);
	if (reaped == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::sys::impl::wait_status_of(raw);
#endif
}
