#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/umount.hpp"
#include "3-system/file/umount_flags.hpp"
#include "3-system/syscall/syscalls.hpp"

#if ca_platform(linux)
#	include <sys/mount.h>
#endif


#if ca_platform(linux)
static_assert(ca::sys::umount_flags::force == MNT_FORCE);
static_assert(ca::sys::umount_flags::detach == MNT_DETACH);
static_assert(ca::sys::umount_flags::expire == MNT_EXPIRE);
static_assert(ca::sys::umount_flags::nofollow == UMOUNT_NOFOLLOW);
#endif

[[nodiscard]] auto ca::sys::umount(const char* const target, const int flags) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::umount2(target, flags).transform([](ca::isz) {});
#elif ca_platform(linux)
	if (::umount2(target, flags) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#else
	static_cast<void>(target);
	static_cast<void>(flags);
	return ca::failure{ca::errc::not_supported};
#endif
}
