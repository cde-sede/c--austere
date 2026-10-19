#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/fdopendir.hpp"
#include "3-system/file/opendir.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <dirent.h>
#include <fcntl.h>


[[nodiscard]] auto ca::sys::opendir(const char* const path) noexcept -> ca::result<ca::sys::directory_stream*> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto fd = ca::syscall::restarting([path] {
		return ca::syscall::openat(AT_FDCWD, path, O_RDONLY | O_DIRECTORY | O_NONBLOCK | O_CLOEXEC, 0);
	});
	if (!fd) [[unlikely]]
		return fd.propagate();
	auto stream = ca::sys::fdopendir(static_cast<int>(*fd));
	if (!stream) [[unlikely]]
		static_cast<void>(ca::sys::close(static_cast<int>(*fd)));
	return stream;
#else
	DIR* const stream = ::opendir(path);
	if (stream == nullptr) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return reinterpret_cast<ca::sys::directory_stream*>(stream);
#endif
}
