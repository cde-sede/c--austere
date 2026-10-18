#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/pread.hpp"

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::pread(const int fd, const ca::span<ca::byte> buffer, const ca::u64 offset) noexcept -> ca::result<ca::usz> {

	ssize_t done{-1};
	do
		done = ::pread(fd, buffer.data(), buffer.size(), static_cast<off_t>(offset));
	while (done == -1 && errno == EINTR);
	if (done == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::usz>(done);
}
