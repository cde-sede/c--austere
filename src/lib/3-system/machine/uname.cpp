#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/machine/uname.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/utsname.h>


[[nodiscard]] auto ca::sys::uname(void) noexcept -> ca::result<ca::sys::system_name> {

	utsname raw{};
#if ca_platform(linux) && ca_architecture(x86_64)
	static_assert(sizeof(utsname) == 6U * 65U, "libc's struct utsname is the kernel's new_utsname");
	const auto done = ca::syscall::uname(&raw);
	if (!done) [[unlikely]]
		return done.propagate();
#else
	if (::uname(&raw) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
#endif
	return ca::sys::system_name{
		ca::sv::str{raw.sysname},
		ca::sv::str{raw.nodename},
		ca::sv::str{raw.release},
		ca::sv::str{raw.version},
		ca::sv::str{raw.machine},
	};
}
