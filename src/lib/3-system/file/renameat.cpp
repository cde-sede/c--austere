#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/renameat.hpp"

#include <fcntl.h>
#include <stdio.h>


[[nodiscard]] auto ca::sys::renameat(const int from_dir, const char* const from, const int to_dir, const char* const to) noexcept -> ca::result<void> {

	if (::renameat(from_dir, from, to_dir, to) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
