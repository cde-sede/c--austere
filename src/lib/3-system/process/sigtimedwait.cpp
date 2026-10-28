#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/i64.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/impl/native_signal_info.hpp"
#include "3-system/process/impl/native_signal_set.hpp"
#include "3-system/process/sigtimedwait.hpp"
#include "3-system/syscall/syscalls.hpp"
#include "3-system/time/now.hpp"

#include <errno.h>
#include <signal.h>
#include <time.h>


[[nodiscard]] auto ca::sys::sigtimedwait(const ca::sys::signal_set& set, const ca::i64 nanoseconds) noexcept -> ca::result<ca::sys::signal_info> {

#if ca_platform(linux)
	sigset_t wanted{};
	ca::sys::impl::to_native(set, &wanted);
	siginfo_t info{};
	const ca::i64 deadline = ca::sys::now(ca::sys::clock::monotonic) + (nanoseconds > 0 ? nanoseconds : 0);
	ca::i64 left = nanoseconds > 0 ? nanoseconds : 0;
	for (;;) {
		const timespec timeout{static_cast<time_t>(left / 1000000000), static_cast<long>(left % 1000000000)};
#	if ca_architecture(x86_64)
		const auto taken = ca::syscall::rt_sigtimedwait(&wanted, &info, &timeout, 8);
		if (taken)
			return ca::sys::impl::from_native_info(&info);
		if (taken.error() == ca::errc::try_again)
			return ca::failure{ca::errc::timed_out};
		if (taken.error() != ca::errc::interrupted) [[unlikely]]
			return taken.propagate();
#	else
		if (::sigtimedwait(&wanted, &info, &timeout) != -1)
			return ca::sys::impl::from_native_info(&info);
		if (errno == EAGAIN)
			return ca::failure{ca::errc::timed_out};
		if (errno != EINTR) [[unlikely]]
			return ca::failure{ca::sys::impl::libc_error()};
#	endif
		left = deadline - ca::sys::now(ca::sys::clock::monotonic);
		if (left < 0)
			left = 0;
	}
#else
	static_cast<void>(set);
	static_cast<void>(nanoseconds);
	return ca::failure{ca::errc::not_supported};
#endif
}
