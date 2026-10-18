#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/impl/status_of.hpp"
#include "3-system/file/fstat.hpp"

#include <sys/stat.h>


[[nodiscard]] auto ca::sys::fstat(const int fd) noexcept -> ca::result<ca::sys::file_status> {

	struct ::stat raw{};
	if (::fstat(fd, &raw) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::sys::impl::status_of(&raw);
}
