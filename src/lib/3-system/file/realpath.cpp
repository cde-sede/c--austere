#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/realpath.hpp"

#include <stdlib.h>


[[nodiscard]] auto ca::sys::realpath(const char* const path) noexcept -> ca::result<char*> {

	char* const resolved = ::realpath(path, nullptr);
	if (resolved == nullptr) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return resolved;
}
