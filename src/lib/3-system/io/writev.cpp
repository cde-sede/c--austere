#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/io/writev.hpp"

#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>


[[nodiscard]] auto ca::sys::writev(const int fd, const ca::span<const ca::sys::io_buffer> buffers) noexcept -> ca::result<ca::usz> {

	if (buffers.size() > 0x7FFFFFFFU) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	ssize_t done{-1};
	do
		done = ::writev(fd, reinterpret_cast<const ::iovec*>(buffers.data()), static_cast<int>(buffers.size()));
	while (done == -1 && errno == EINTR);
	if (done == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::usz>(done);
}
