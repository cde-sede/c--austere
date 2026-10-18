#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/rename_noreplace.hpp"

#include <fcntl.h>
#include <stdio.h>


[[nodiscard]] auto ca::sys::rename_noreplace(const char* const from, const char* const to) noexcept -> ca::result<void> {

#if ca_platform(linux)
	if (::renameat2(AT_FDCWD, from, AT_FDCWD, to, RENAME_NOREPLACE) == -1) [[unlikely]]
#else
	if (::renamex_np(from, to, RENAME_EXCL) == -1) [[unlikely]]
#endif
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
