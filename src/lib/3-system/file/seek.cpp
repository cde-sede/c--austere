#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/seek.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::seek(const int fd, const ca::i64 offset, const ca::sys::seek_origin origin) noexcept -> ca::result<ca::i64> {

	const int whence = origin == ca::sys::seek_origin::start ? SEEK_SET : origin == ca::sys::seek_origin::current ? SEEK_CUR : SEEK_END;
#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::lseek(fd, offset, whence).transform([](const ca::isz at) {
		return static_cast<ca::i64>(at);
	});
#else
	const off_t at = ::lseek(fd, static_cast<off_t>(offset), whence);
	if (at == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::i64>(at);
#endif
}
