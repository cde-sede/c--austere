#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/chmod.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/stat.h>


[[nodiscard]] auto ca::sys::chmod(const char* const path, const ca::u32 mode) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::chmod(path, mode).transform([](ca::isz) {});
#else
	if (::chmod(path, static_cast<mode_t>(mode)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
