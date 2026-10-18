#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/read.hpp"

#include <errno.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::read(const int fd, const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {

	ssize_t done{-1};
	do
		done = ::read(fd, buffer.data(), buffer.size());
	while (done == -1 && errno == EINTR);
	if (done == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::usz>(done);
}
