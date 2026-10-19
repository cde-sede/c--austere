#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/linkat.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <fcntl.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::linkat(const int from_dir, const char* const from, const int to_dir, const char* const to, const int flags) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::linkat(from_dir, from, to_dir, to, flags).transform([](ca::isz) {});
#else
	if (::linkat(from_dir, from, to_dir, to, flags) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
