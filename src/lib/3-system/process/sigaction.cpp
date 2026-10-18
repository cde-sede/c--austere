#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/sigaction.hpp"

#include <signal.h>


[[nodiscard]] static auto install(const int signal, void (*const handler)(int)) noexcept -> ca::result<void> {

	struct ::sigaction action{};
	action.sa_handler = handler;
	::sigemptyset(&action.sa_mask);
	action.sa_flags = SA_RESTART;
	if (::sigaction(signal, &action, nullptr) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}

[[nodiscard]] auto ca::sys::sigaction(const int signal, void (*const handler)(int)) noexcept -> ca::result<void> {
	return install(signal, handler);
}

[[nodiscard]] auto ca::sys::sigaction(const int signal, const ca::sys::signal_disposition disposition) noexcept -> ca::result<void> {
	return install(signal, disposition == ca::sys::signal_disposition::ignore ? SIG_IGN : SIG_DFL);
}
