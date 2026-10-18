#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/raise.hpp"

#include <signal.h>


[[nodiscard]] auto ca::sys::raise(const int signal) noexcept -> ca::result<void> {

	if (::raise(signal) != 0) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
