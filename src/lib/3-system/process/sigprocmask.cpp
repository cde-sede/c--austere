#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/process/impl/native_signal_set.hpp"
#include "3-system/process/sigprocmask.hpp"

#include <pthread.h>
#include <signal.h>


[[nodiscard]] auto ca::sys::sigprocmask(const ca::sys::mask_change how, const ca::sys::signal_set& set) noexcept -> ca::result<ca::sys::signal_set> {

	const int native_how = how == ca::sys::mask_change::block ? SIG_BLOCK
		: how == ca::sys::mask_change::unblock ? SIG_UNBLOCK : SIG_SETMASK;
	sigset_t wanted{};
	sigset_t previous{};
	ca::sys::impl::to_native(set, &wanted);
	const int code = ::pthread_sigmask(native_how, &wanted, &previous);
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	return ca::sys::impl::from_native(&previous);
}
