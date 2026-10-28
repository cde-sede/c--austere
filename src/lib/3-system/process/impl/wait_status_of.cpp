#include "3-system/process/impl/wait_status_of.hpp"

#include <sys/wait.h>


[[nodiscard]] auto ca::sys::impl::wait_status_of(const ca::i32 child, const int raw) noexcept -> ca::sys::wait_status {

	if (WIFEXITED(raw))
		return ca::sys::wait_status{child, true, WEXITSTATUS(raw)};
	return ca::sys::wait_status{child, false, WTERMSIG(raw)};
}
