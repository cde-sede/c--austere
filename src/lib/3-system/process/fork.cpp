#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/fork.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::fork(void) noexcept -> ca::result<ca::i32> {

	const pid_t child = ::fork();
	if (child == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::i32>(child);
}
