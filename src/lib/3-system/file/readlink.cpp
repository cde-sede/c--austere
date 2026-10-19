#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/readlink.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::readlink(const char* const path, const ca::span<char> buffer) noexcept -> ca::result<ca::usz> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::readlink(path, buffer.data(), buffer.size()).transform([](const ca::isz length) {
		return static_cast<ca::usz>(length);
	});
#else
	const ssize_t length = ::readlink(path, buffer.data(), buffer.size());
	if (length == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::usz>(length);
#endif
}
