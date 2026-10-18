#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/setsid.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::setsid(void) noexcept -> ca::result<ca::i32> {

	const pid_t session = ::setsid();
	if (session == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::i32>(session);
}
