// expect: exit 0
// link: both
#include "3-system/error/errc.hpp"
#include "3-system/file/at.hpp"
#include "3-system/file/mkdirat.hpp"
#include "3-system/file/mount.hpp"
#include "3-system/file/mount_flags.hpp"
#include "3-system/file/umount.hpp"
#include "3-system/file/umount_flags.hpp"
#include "3-system/file/unlinkat.hpp"

auto main(void) -> int {
	static_cast<void>(ca::sys::umount("build/sys_mount.tmp", ca::sys::umount_flags::detach));
	static_cast<void>(ca::sys::unlinkat(ca::sys::at::cwd, "build/sys_mount.tmp", ca::sys::at::remove_directory));

	auto missing = ca::sys::mount("none", "build/sys_mount.missing", "tmpfs", ca::sys::mount_flags::nosuid, nullptr);
	if (missing || missing.error() != ca::errc::no_entry)
		return 1;
	auto unknown = ca::sys::umount("build", 0x100);
	if (unknown || unknown.error() != ca::errc::invalid_argument)
		return 2;
	auto plain = ca::sys::umount("build", 0);
	if (plain || (plain.error() != ca::errc::not_permitted && plain.error() != ca::errc::invalid_argument))
		return 3;

	if (!ca::sys::mkdirat(ca::sys::at::cwd, "build/sys_mount.tmp", 0755U))
		return 4;
	constexpr ca::u64 flags = ca::sys::mount_flags::nosuid | ca::sys::mount_flags::nodev | ca::sys::mount_flags::noexec;
	auto mounted = ca::sys::mount("none", "build/sys_mount.tmp", "tmpfs", flags, "size=64k");
	if (mounted) {
		auto remount = ca::sys::mount(nullptr, "build/sys_mount.tmp", nullptr, ca::sys::mount_flags::remount | ca::sys::mount_flags::read_only | flags, nullptr);
		if (!remount || !ca::sys::umount("build/sys_mount.tmp", 0))
			return 5;
	} else if (mounted.error() != ca::errc::not_permitted)
		return 6;
	return ca::sys::unlinkat(ca::sys::at::cwd, "build/sys_mount.tmp", ca::sys::at::remove_directory) ? 0 : 7;
}
