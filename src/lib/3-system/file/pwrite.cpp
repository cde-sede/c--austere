#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/pwrite.hpp"

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::pwrite(const int fd, const ca::span<const ca::byte> bytes, const ca::u64 offset) noexcept -> ca::result<ca::usz> {

	ssize_t done{-1};
	do
		done = ::pwrite(fd, bytes.data(), bytes.size(), static_cast<off_t>(offset));
	while (done == -1 && errno == EINTR);
	if (done == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::usz>(done);
}
