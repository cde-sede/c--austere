#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/closedir.hpp"

#include <dirent.h>


[[nodiscard]] auto ca::sys::closedir(ca::sys::directory_stream* const stream) noexcept -> ca::result<void> {

	if (::closedir(reinterpret_cast<DIR*>(stream)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
