#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/setpgid.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::setpgid(const ca::i32 pid, const ca::i32 group) noexcept -> ca::result<void> {

	if (::setpgid(static_cast<pid_t>(pid), static_cast<pid_t>(group)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
