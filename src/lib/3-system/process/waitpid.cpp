#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/impl/wait_status_of.hpp"
#include "3-system/process/waitpid.hpp"

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>


[[nodiscard]] auto ca::sys::waitpid(const ca::i32 pid) noexcept -> ca::result<ca::sys::wait_status> {

	int raw{0};
	pid_t reaped{-1};
	do
		reaped = ::waitpid(static_cast<pid_t>(pid), &raw, 0);
	while (reaped == -1 && errno == EINTR);
	if (reaped == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::sys::impl::wait_status_of(raw);
}
