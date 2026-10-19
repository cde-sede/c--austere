#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/process/impl/native_signal_set.hpp"
#include "3-system/process/sigprocmask.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <pthread.h>
#include <signal.h>


[[nodiscard]] auto ca::sys::sigprocmask(const ca::sys::mask_change how, const ca::sys::signal_set& set) noexcept -> ca::result<ca::sys::signal_set> {

	const int native_how = how == ca::sys::mask_change::block ? SIG_BLOCK
		: how == ca::sys::mask_change::unblock ? SIG_UNBLOCK : SIG_SETMASK;
	sigset_t wanted{};
	sigset_t previous{};
#if ca_platform(linux) && ca_architecture(x86_64)
	ca::sys::signal_set kept = set;
	if (how != ca::sys::mask_change::unblock) {
		kept.remove(32);
		kept.remove(33);
	}
	ca::sys::impl::to_native(kept, &wanted);
	const auto done = ca::syscall::rt_sigprocmask(native_how, &wanted, &previous, 8);
	if (!done) [[unlikely]]
		return done.propagate();
#else
	ca::sys::impl::to_native(set, &wanted);
	const int code = ::pthread_sigmask(native_how, &wanted, &previous);
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
#endif
	return ca::sys::impl::from_native(&previous);
}
