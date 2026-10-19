#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/write.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <errno.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::write(const int fd, const ca::span<const ca::byte> bytes) noexcept -> ca::result<ca::usz> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::restarting([&] {
		return ca::syscall::write(fd, bytes.data(), bytes.size());
	}).transform([](const ca::isz done) {
		return static_cast<ca::usz>(done);
	});
#else
	ssize_t done{-1};
	do
		done = ::write(fd, bytes.data(), bytes.size());
	while (done == -1 && errno == EINTR);
	if (done == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::usz>(done);
#endif
}
