#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/renameat.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <fcntl.h>
#include <stdio.h>


[[nodiscard]] auto ca::sys::renameat(const int from_dir, const char* const from, const int to_dir, const char* const to) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::renameat(from_dir, from, to_dir, to).transform([](ca::isz) {});
#else
	if (::renameat(from_dir, from, to_dir, to) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
