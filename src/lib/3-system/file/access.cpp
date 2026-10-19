#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/access.hpp"
#include "3-system/file/access_checks.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <fcntl.h>
#include <unistd.h>


static_assert(ca::sys::access_checks::exists == F_OK);
static_assert(ca::sys::access_checks::execute == X_OK);
static_assert(ca::sys::access_checks::write == W_OK);
static_assert(ca::sys::access_checks::read == R_OK);

[[nodiscard]] auto ca::sys::access(const char* const path, const int checks) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::faccessat(AT_FDCWD, path, checks).transform([](ca::isz) {});
#else
	if (::access(path, checks) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
