#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/process/impl/native_signal_set.hpp"
#include "3-system/process/sigwait.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <signal.h>


[[nodiscard]] auto ca::sys::sigwait(const ca::sys::signal_set& set) noexcept -> ca::result<int> {

	sigset_t wanted{};
	ca::sys::impl::to_native(set, &wanted);
#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::restarting([&wanted] {
		return ca::syscall::rt_sigtimedwait(&wanted, nullptr, nullptr, 8);
	}).transform([](const ca::isz signal) {
		return static_cast<int>(signal);
	});
#else
	int signal{0};
	const int code = ::sigwait(&wanted, &signal);
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	return signal;
#endif
}
