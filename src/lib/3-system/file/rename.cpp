#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/rename.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <stdio.h>


[[nodiscard]] auto ca::sys::rename(const char* const from, const char* const to) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::rename(from, to).transform([](ca::isz) {});
#else
	if (::rename(from, to) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
