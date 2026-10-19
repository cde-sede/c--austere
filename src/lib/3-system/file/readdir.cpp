#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u16.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/impl/directory_stream.hpp"
#include "3-system/file/readdir.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <dirent.h>
#include <errno.h>


#if ca_platform(linux) && ca_architecture(x86_64)

static constexpr ca::usz RECORD_LENGTH = 16U;
static constexpr ca::usz NAME = 19U;

/* An entry as getdents64 writes it (struct linux_dirent64): inode, offset, record length at 16,
   type at 18, the NUL-terminated name at 19. */
static auto record_length(const char* const entry) noexcept -> ca::u16 {

	ca::u16 length;
	__builtin_memcpy(&length, entry + RECORD_LENGTH, sizeof(length));
	return length;
}

[[nodiscard]] auto ca::sys::readdir(ca::sys::directory_stream* const stream) noexcept -> ca::result<ca::sv::str> {
	for (;;) {
		if (stream->_at >= stream->_end) {
			const auto filled = ca::syscall::restarting([stream] {
				return ca::syscall::getdents64(stream->_fd, stream->_entries, sizeof(stream->_entries));
			});
			if (!filled) [[unlikely]]
				return filled.propagate();
			if (*filled == 0)
				return ca::sv::str{};
			 stream->_at = 0U;
			stream->_end = static_cast<ca::u32>(*filled);
		}
		const char* const entry = stream->_entries + stream->_at;
		stream->_at += record_length(entry);
		const ca::sv::str name{entry + NAME};
		if (name != ca::sv::str{"."} && name != ca::sv::str{".."})
			return name;
	}
}

#else

[[nodiscard]] auto ca::sys::readdir(ca::sys::directory_stream* const stream) noexcept -> ca::result<ca::sv::str> {
	for (;;) {
		errno = 0;
		const dirent* const entry = ::readdir(reinterpret_cast<DIR*>(stream));
		if (entry == nullptr) {
			if (errno != 0) [[unlikely]]
				return ca::failure{ca::sys::impl::libc_error()};
			return ca::sv::str{};
		}
		const ca::sv::str name{entry->d_name};
		if (name != ca::sv::str{"."} && name != ca::sv::str{".."})
			return name;
	}
}

#endif
