#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/process/impl/native_signal_set.hpp"
#include "3-system/process/sigwait.hpp"

#include <signal.h>


[[nodiscard]] auto ca::sys::sigwait(const ca::sys::signal_set& set) noexcept -> ca::result<int> {

	sigset_t wanted{};
	ca::sys::impl::to_native(set, &wanted);
	int signal{0};
	const int code = ::sigwait(&wanted, &signal);
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	return signal;
}
