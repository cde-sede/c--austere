#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/fcntl.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <fcntl.h>


[[nodiscard]] auto ca::sys::fcntl(const int fd, const int command) noexcept -> ca::result<int> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::fcntl(fd, command, 0).transform([](const ca::isz value) {
		return static_cast<int>(value);
	});
#else
	const int value = ::fcntl(fd, command);
	if (value == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return value;
#endif
}

[[nodiscard]] auto ca::sys::fcntl(const int fd, const int command, const int argument) noexcept -> ca::result<int> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::fcntl(fd, command, argument).transform([](const ca::isz value) {
		return static_cast<int>(value);
	});
#else
	const int value = ::fcntl(fd, command, argument);
	if (value == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return value;
#endif
}
