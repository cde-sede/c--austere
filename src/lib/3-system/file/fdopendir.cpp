#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/fdopendir.hpp"
#include "3-system/file/file_kind.hpp"
#include "3-system/file/fstat.hpp"
#include "3-system/file/impl/directory_stream.hpp"
#include "3-system/memory/mmap_anonymous.hpp"
#include "3-system/memory/protection.hpp"

#include <dirent.h>


[[nodiscard]] auto ca::sys::fdopendir(const int fd) noexcept -> ca::result<ca::sys::directory_stream*> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto status = ca::sys::fstat(fd);
	if (!status) [[unlikely]]
		return status.propagate();
	if (status->kind() != ca::sys::file_kind::directory) [[unlikely]] {
		ca::sys::set_last_error(ca::errc::not_a_directory);
		return ca::failure{ca::errc::not_a_directory};
	}
	const auto mapping = ca::sys::mmap_anonymous(ca::sys::directory_stream::SIZE, ca::sys::protection::read | ca::sys::protection::write);
	if (!mapping) [[unlikely]]
		return mapping.propagate();
	ca::sys::directory_stream* const stream = reinterpret_cast<ca::sys::directory_stream*>(mapping->data());
	 stream->_fd = fd;
	 stream->_at = 0U;
	stream->_end = 0U;
	return stream;
#else
	DIR* const stream = ::fdopendir(fd);
	if (stream == nullptr) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return reinterpret_cast<ca::sys::directory_stream*>(stream);
#endif
}
