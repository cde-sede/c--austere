#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/rmdir.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::rmdir(const char* const path) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::rmdir(path).transform([](ca::isz) {});
#else
	if (::rmdir(path) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
