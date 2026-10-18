#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/opendir.hpp"

#include <dirent.h>


[[nodiscard]] auto ca::sys::opendir(const char* const path) noexcept -> ca::result<ca::sys::directory_stream*> {

	DIR* const stream = ::opendir(path);
	if (stream == nullptr) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return reinterpret_cast<ca::sys::directory_stream*>(stream);
}
