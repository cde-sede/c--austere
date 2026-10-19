#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/read.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <errno.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::read(const int fd, const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::restarting([&] {
		return ca::syscall::read(fd, buffer.data(), buffer.size());
	}).transform([](const ca::isz done) {
		return static_cast<ca::usz>(done);
	});
#else
	ssize_t done{-1};
	do
		done = ::read(fd, buffer.data(), buffer.size());
	while (done == -1 && errno == EINTR);
	if (done == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::usz>(done);
#endif
}
