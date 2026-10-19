#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/faccessat.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <fcntl.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::faccessat(const int dir, const char* const path, const int checks, const int flags) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	if (flags == 0)
		return ca::syscall::faccessat(dir, path, checks).transform([](ca::isz) {});
	return ca::syscall::faccessat2(dir, path, checks, flags).transform([](ca::isz) {});
#else
	if (::faccessat(dir, path, checks, flags) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
