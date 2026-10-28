#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/mount.hpp"
#include "3-system/file/mount_flags.hpp"
#include "3-system/syscall/syscalls.hpp"

#if ca_platform(linux)
#	include <sys/mount.h>
#endif


#if ca_platform(linux)
static_assert(ca::sys::mount_flags::read_only == MS_RDONLY);
static_assert(ca::sys::mount_flags::nosuid == MS_NOSUID);
static_assert(ca::sys::mount_flags::nodev == MS_NODEV);
static_assert(ca::sys::mount_flags::noexec == MS_NOEXEC);
static_assert(ca::sys::mount_flags::synchronous == MS_SYNCHRONOUS);
static_assert(ca::sys::mount_flags::remount == MS_REMOUNT);
static_assert(ca::sys::mount_flags::mandatory_lock == MS_MANDLOCK);
static_assert(ca::sys::mount_flags::directory_sync == MS_DIRSYNC);
#	if defined(MS_NOSYMFOLLOW)
static_assert(ca::sys::mount_flags::nosymfollow == MS_NOSYMFOLLOW);
#	endif
static_assert(ca::sys::mount_flags::noatime == MS_NOATIME);
static_assert(ca::sys::mount_flags::nodiratime == MS_NODIRATIME);
static_assert(ca::sys::mount_flags::bind == MS_BIND);
static_assert(ca::sys::mount_flags::move == MS_MOVE);
static_assert(ca::sys::mount_flags::recursive == MS_REC);
static_assert(ca::sys::mount_flags::silent == MS_SILENT);
static_assert(ca::sys::mount_flags::propagation_unbindable == MS_UNBINDABLE);
static_assert(ca::sys::mount_flags::propagation_private == MS_PRIVATE);
static_assert(ca::sys::mount_flags::propagation_slave == MS_SLAVE);
static_assert(ca::sys::mount_flags::propagation_shared == MS_SHARED);
static_assert(ca::sys::mount_flags::relatime == MS_RELATIME);
static_assert(ca::sys::mount_flags::strictatime == MS_STRICTATIME);
static_assert(ca::sys::mount_flags::lazytime == MS_LAZYTIME);
#endif

[[nodiscard]] auto ca::sys::mount(const char* const source, const char* const target, const char* const type, const ca::u64 flags, const void* const data) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::mount(source, target, type, flags, data).transform([](ca::isz) {});
#elif ca_platform(linux)
	if (::mount(source, target, type, static_cast<unsigned long>(flags), data) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#else
	static_cast<void>(source);
	static_cast<void>(target);
	static_cast<void>(type);
	static_cast<void>(flags);
	static_cast<void>(data);
	return ca::failure{ca::errc::not_supported};
#endif
}
