#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/i64.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/sigqueue.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <signal.h>
#include <sys/types.h>


[[nodiscard]] auto ca::sys::sigqueue(const ca::i32 pid, const int signal, const ca::i64 value) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	siginfo_t info{};
	          info.si_signo = signal;
	           info.si_code = SI_QUEUE;
	            info.si_pid = static_cast<pid_t>(ca::syscall::getpid().value_or(0));
	            info.si_uid = static_cast<uid_t>(ca::syscall::getuid().value_or(0));
	info.si_value.sival_ptr = reinterpret_cast<void*>(static_cast<__INTPTR_TYPE__>(value));
	return ca::syscall::rt_sigqueueinfo(pid, signal, &info).transform([](ca::isz) {});
#elif ca_platform(linux)
	union sigval carried{};
	carried.sival_ptr = reinterpret_cast<void*>(static_cast<__INTPTR_TYPE__>(value));
	if (::sigqueue(static_cast<pid_t>(pid), signal, carried) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#else
	static_cast<void>(pid);
	static_cast<void>(signal);
	static_cast<void>(value);
	return ca::failure{ca::errc::not_supported};
#endif
}
