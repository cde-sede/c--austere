#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/linkat.hpp"

#include <fcntl.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::linkat(const int from_dir, const char* const from, const int to_dir, const char* const to, const int flags) noexcept -> ca::result<void> {

	if (::linkat(from_dir, from, to_dir, to, flags) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
