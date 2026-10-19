#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/dirfd.hpp"
#include "3-system/file/impl/directory_stream.hpp"

#include <dirent.h>


[[nodiscard]] auto ca::sys::dirfd(ca::sys::directory_stream* const stream) noexcept -> ca::result<int> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return stream->_fd;
#else
	const int fd = ::dirfd(reinterpret_cast<DIR*>(stream));
	if (fd == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return fd;
#endif
}
