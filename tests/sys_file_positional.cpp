// expect: exit 0
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/access.hpp"
#include "3-system/file/access_checks.hpp"
#include "3-system/file/at.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/faccessat.hpp"
#include "3-system/file/fallocate.hpp"
#include "3-system/file/fchmod.hpp"
#include "3-system/file/fdatasync.hpp"
#include "3-system/file/fstat.hpp"
#include "3-system/file/ftruncate.hpp"
#include "3-system/file/link.hpp"
#include "3-system/file/mkdtemp.hpp"
#include "3-system/file/mkstemp.hpp"
#include "3-system/file/pread.hpp"
#include "3-system/file/pwrite.hpp"
#include "3-system/file/rename_noreplace.hpp"
#include "3-system/file/rmdir.hpp"
#include "3-system/file/seek.hpp"
#include "3-system/file/seek_origin.hpp"
#include "3-system/file/stat.hpp"
#include "3-system/file/truncate.hpp"
#include "3-system/file/umask.hpp"
#include "3-system/file/unlink.hpp"
#include "3-system/fs/options.hpp"
#include "3-system/io/io_buffer.hpp"
#include "3-system/io/readv.hpp"
#include "3-system/io/writev.hpp"

static auto same(const char* a, const char* b, const ca::usz size) noexcept -> bool {

	for (ca::usz i = 0U; i < size; ++i) {
		if (a[i] != b[i])
			return false;
	}
	return true;
}

static auto size_of(const int fd) noexcept -> ca::u64 {

	auto status = ca::sys::fstat(fd);
	return status ? status->size() : 0xFFFFFFFFU;
}

static auto links_of(const char* const path) noexcept -> ca::u64 {

	auto status = ca::sys::stat(path);
	return status ? status->links() : 0U;
}

static auto mode_of(const int fd) noexcept -> ca::u32 {

	auto status = ca::sys::fstat(fd);
	return status ? status->permissions() : 0U;
}

auto main(void) -> int {
	char dir[] = "build/sys_file_positional.XXXXXX";
	if (!ca::sys::mkdtemp(dir) || same(dir + 26, "XXXXXX", 6U))
		return 1;
	char file[] = "build/sys_file_positional.XXXXXX/f.XXXXXX";
	for (ca::usz i = 0U; i < 6U; ++i)
		file[26 + i] = dir[26 + i];
	auto fd = ca::sys::mkstemp(file, ca::fs::cloexec_t::OPEN_VALUE);
	if (!fd)
		return 2;
	if (mode_of(*fd) != 0600U)
		return 3;

	const ca::sys::io_buffer out[3]{ca::sv::str{"ab"}, ca::sv::str{""}, ca::sv::str{"cdef"}};
	auto written = ca::sys::writev(*fd, out);
	if (!written || *written != 6U)
		return 4;
	auto patched = ca::sys::pwrite(*fd, ca::sv::str{"XY"}, 1U);
	if (!patched || *patched != 2U)
		return 5;
	char got[8]{};
	auto read = ca::sys::pread(*fd, got, 0U);
	if (!read || *read != 6U || !same(got, "aXYdef", 6U))
		return 6;
	auto past = ca::sys::pread(*fd, got, 100U);
	if (!past || *past != 0U)
		return 7;
	char head[3]{};
	char tail[3]{};
	const ca::sys::io_buffer in[2]{head, tail};
	auto gathered = ca::sys::seek(*fd, 0, ca::sys::seek_origin::start).and_then([&](ca::i64) {
		return ca::sys::readv(*fd, in);
	});
	if (!gathered || *gathered != 6U || !same(head, "aXY", 3U) || !same(tail, "def", 3U))
		return 8;

	if (!ca::sys::ftruncate(*fd, 3U) || size_of(*fd) != 3U)
		return 9;
	if (!ca::sys::truncate(file, 10U) || size_of(*fd) != 10U)
		return 10;
	if (!ca::sys::fallocate(*fd, 0U, 65536U) || size_of(*fd) != 65536U || !ca::sys::fdatasync(*fd))
		return 11;
	if (!ca::sys::fallocate(*fd, 0U, 10U) || size_of(*fd) != 65536U)
		return 12;

	if (!ca::sys::fchmod(*fd, 0640U) || mode_of(*fd) != 0640U)
		return 13;
	if (!ca::sys::access(file, ca::sys::access_checks::read | ca::sys::access_checks::write))
		return 14;
	auto denied = ca::sys::access(file, ca::sys::access_checks::execute);
	if (denied || denied.error() != ca::errc::access_denied)
		return 15;
	auto missing = ca::sys::faccessat(ca::sys::at::cwd, "build/does-not-exist", ca::sys::access_checks::exists, 0);
	if (missing || missing.error() != ca::errc::no_entry)
		return 16;

	const ca::u32 before = ca::sys::umask(077U);
	if (ca::sys::umask(before) != 077U)
		return 17;

	char twin[] = "build/sys_file_positional.XXXXXX/twin";
	char moved[] = "build/sys_file_positional.XXXXXX/moved";
	for (ca::usz i = 0U; i < 6U; ++i) {
		twin[26 + i] = dir[26 + i];
		moved[26 + i] = dir[26 + i];
	}
	if (!ca::sys::link(file, twin) || links_of(file) != 2U)
		return 18;
	auto clash = ca::sys::rename_noreplace(twin, file);
	if (clash || clash.error() != ca::errc::exists)
		return 19;
	if (!ca::sys::rename_noreplace(twin, moved))
		return 20;

	if (!ca::sys::close(*fd) || !ca::sys::unlink(file) || !ca::sys::unlink(moved) || !ca::sys::rmdir(dir))
		return 21;
	return 0;
}
