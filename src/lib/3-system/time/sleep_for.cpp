#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/time/sleep_for.hpp"

#include <errno.h>
#include <time.h>


[[nodiscard]] auto ca::sys::sleep_for(const ca::i64 nanoseconds) noexcept -> ca::result<void> {

	timespec left{static_cast<time_t>(nanoseconds / 1000000000), static_cast<long>(nanoseconds % 1000000000)};
	while (::nanosleep(&left, &left) == -1) {
		if (errno != EINTR) [[unlikely]]
			return ca::failure{ca::sys::impl::libc_error()};
	}
	return ca::result<void>{};
}
