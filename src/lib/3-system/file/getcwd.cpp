#include "1-primitives/memory/string_length.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/getcwd.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::getcwd(const ca::span<char> buffer) noexcept -> ca::result<ca::usz> {

	if (::getcwd(buffer.data(), buffer.size()) == nullptr) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::string_length(buffer.data());
}
