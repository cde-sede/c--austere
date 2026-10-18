#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/fcntl.hpp"

#include <fcntl.h>


[[nodiscard]] auto ca::sys::fcntl(const int fd, const int command) noexcept -> ca::result<int> {

	const int value = ::fcntl(fd, command);
	if (value == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return value;
}

[[nodiscard]] auto ca::sys::fcntl(const int fd, const int command, const int argument) noexcept -> ca::result<int> {

	const int value = ::fcntl(fd, command, argument);
	if (value == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return value;
}
