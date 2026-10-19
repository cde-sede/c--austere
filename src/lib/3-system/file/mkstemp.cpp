#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/impl/temporary_name.hpp"
#include "3-system/file/mkstemp.hpp"
#include "3-system/file/open.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::mkstemp(char* const pattern, const int flags) noexcept -> ca::result<int> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto suffix = ca::sys::impl::temporary_suffix(pattern);
	if (!suffix) [[unlikely]]
		return suffix.propagate();
	for (int attempt = 0; attempt < ca::sys::impl::TEMPORARY_ATTEMPTS; ++attempt) {
		const auto named = ca::sys::impl::randomize_suffix(*suffix);
		if (!named) [[unlikely]]
			return named.propagate();
		auto fd = ca::sys::open(pattern, O_RDWR | O_CREAT | O_EXCL | flags, 0600U);
		if (fd || fd.error() != ca::errc::exists)
			return fd;
	}
	return ca::failure{ca::errc::exists};
#else
	int fd{-1};
	do
		fd = ::mkostemp(pattern, flags);
	while (fd == -1 && errno == EINTR);
	if (fd == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return fd;
#endif
}
