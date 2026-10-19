#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/closedir.hpp"
#include "3-system/file/impl/directory_stream.hpp"
#include "3-system/memory/munmap.hpp"

#include <dirent.h>


[[nodiscard]] auto ca::sys::closedir(ca::sys::directory_stream* const stream) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	auto closed = ca::sys::close(stream->_fd);
	static_cast<void>(ca::sys::munmap(ca::span<ca::byte>{reinterpret_cast<ca::byte*>(stream), ca::sys::directory_stream::SIZE}));
	return closed;
#else
	if (::closedir(reinterpret_cast<DIR*>(stream)) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
