#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/impl/native_signal_info.hpp"
#include "3-system/process/impl/native_signal_set.hpp"
#include "3-system/process/sigwaitinfo.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <errno.h>
#include <signal.h>


[[nodiscard]] auto ca::sys::sigwaitinfo(const ca::sys::signal_set& set) noexcept -> ca::result<ca::sys::signal_info> {

#if ca_platform(linux)
	sigset_t wanted{};
	ca::sys::impl::to_native(set, &wanted);
	siginfo_t info{};
#	if ca_architecture(x86_64)
	const auto taken = ca::syscall::restarting([&wanted, &info] {
		return ca::syscall::rt_sigtimedwait(&wanted, &info, nullptr, 8);
	});
	if (!taken) [[unlikely]]
		return taken.propagate();
#	else
	while (::sigwaitinfo(&wanted, &info) == -1) {
		if (errno != EINTR) [[unlikely]]
			return ca::failure{ca::sys::impl::libc_error()};
	}
#	endif
	return ca::sys::impl::from_native_info(&info);
#else
	static_cast<void>(set);
	return ca::failure{ca::errc::not_supported};
#endif
}
