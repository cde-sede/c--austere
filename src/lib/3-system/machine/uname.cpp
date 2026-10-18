#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/machine/uname.hpp"

#include <sys/utsname.h>


[[nodiscard]] auto ca::sys::uname(void) noexcept -> ca::result<ca::sys::system_name> {

	utsname raw{};
	if (::uname(&raw) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::sys::system_name{
		ca::sv::str{raw.sysname},
		ca::sv::str{raw.nodename},
		ca::sv::str{raw.release},
		ca::sv::str{raw.version},
		ca::sv::str{raw.machine},
	};
}
