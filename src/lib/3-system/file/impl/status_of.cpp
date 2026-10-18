#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "3-system/file/file_kind.hpp"
#include "3-system/file/impl/status_of.hpp"

#include <sys/stat.h>


static auto kind_of(const mode_t mode) noexcept -> ca::sys::file_kind {
	switch (mode & S_IFMT) {
		case S_IFREG:
			return ca::sys::file_kind::regular;
		case S_IFDIR:
			return ca::sys::file_kind::directory;
		case S_IFLNK:
			return ca::sys::file_kind::symlink;
		case S_IFIFO:
			return ca::sys::file_kind::fifo;
		case S_IFSOCK:
			return ca::sys::file_kind::socket;
		case S_IFBLK:
			return ca::sys::file_kind::block_device;
		case S_IFCHR:
			return ca::sys::file_kind::character_device;
		default:
			return ca::sys::file_kind::unknown;
	}
}

static auto nanoseconds_of(const timespec& time) noexcept -> ca::i64 {
	return static_cast<ca::i64>(time.tv_sec) * 1000000000 + static_cast<ca::i64>(time.tv_nsec);
}

[[nodiscard]] auto ca::sys::impl::status_of(const void* const raw) noexcept -> ca::sys::file_status {

	const struct ::stat& status = *static_cast<const struct ::stat*>(raw);
	return ca::sys::file_status{
		kind_of(status.st_mode),
		static_cast<ca::u32>(status.st_mode & 07777),
		static_cast<ca::u64>(status.st_size),
		static_cast<ca::u64>(status.st_ino),
		static_cast<ca::u64>(status.st_dev),
		static_cast<ca::u64>(status.st_nlink),
		static_cast<ca::u32>(status.st_uid),
		static_cast<ca::u32>(status.st_gid),
		nanoseconds_of(status.st_atim),
		nanoseconds_of(status.st_mtim),
		nanoseconds_of(status.st_ctim),
	};
}
