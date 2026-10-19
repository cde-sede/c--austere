#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/raise.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <signal.h>


[[nodiscard]] auto ca::sys::raise(const int signal) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const ca::isz process = ca::syscall::getpid().value_or(0);
	const ca::isz thread = ca::syscall::gettid().value_or(0);
	return ca::syscall::tgkill(process, thread, signal).transform([](ca::isz) {});
#else
	if (::raise(signal) != 0) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
