#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/fstatat.hpp"
#include "3-system/file/impl/status_of.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <fcntl.h>
#include <sys/stat.h>


[[nodiscard]] auto ca::sys::fstatat(const int dir, const char* const path, const int flags) noexcept -> ca::result<ca::sys::file_status> {

	struct ::stat raw{};
#if ca_platform(linux) && ca_architecture(x86_64)
	const auto done = ca::syscall::newfstatat(dir, path, &raw, flags);
	if (!done) [[unlikely]]
		return done.propagate();
#else
	if (::fstatat(dir, path, &raw, flags) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
#endif
	return ca::sys::impl::status_of(&raw);
}
