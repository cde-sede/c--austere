#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/truncate.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::truncate(const char* const path, const ca::u64 size) noexcept -> ca::result<void> {

	if (::truncate(path, static_cast<off_t>(size)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
