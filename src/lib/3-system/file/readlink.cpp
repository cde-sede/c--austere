#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/readlink.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::readlink(const char* const path, const ca::span<char> buffer) noexcept -> ca::result<ca::usz> {

	const ssize_t length = ::readlink(path, buffer.data(), buffer.size());
	if (length == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::usz>(length);
}
