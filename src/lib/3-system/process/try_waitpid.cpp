#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/impl/wait_status_of.hpp"
#include "3-system/process/try_waitpid.hpp"

#include <sys/types.h>
#include <sys/wait.h>


[[nodiscard]] auto ca::sys::try_waitpid(const ca::i32 pid) noexcept -> ca::result<ca::optional<ca::sys::wait_status>> {

	int raw{0};
	const pid_t reaped = ::waitpid(static_cast<pid_t>(pid), &raw, WNOHANG);
	if (reaped == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	if (reaped == 0)
		return ca::optional<ca::sys::wait_status>{};
	return ca::optional<ca::sys::wait_status>{ca::sys::impl::wait_status_of(raw)};
}
