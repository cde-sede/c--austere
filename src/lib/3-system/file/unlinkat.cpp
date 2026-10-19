#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/unlinkat.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <fcntl.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::unlinkat(const int dir, const char* const path, const int flags) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::unlinkat(dir, path, flags).transform([](ca::isz) {});
#else
	if (::unlinkat(dir, path, flags) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
