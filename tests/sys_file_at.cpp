// expect: exit 0
#include "3-system/error/errc.hpp"
#include "3-system/file/at.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/closedir.hpp"
#include "3-system/file/dirfd.hpp"
#include "3-system/file/fdopendir.hpp"
#include "3-system/file/fstatat.hpp"
#include "3-system/file/linkat.hpp"
#include "3-system/file/mkdirat.hpp"
#include "3-system/file/openat.hpp"
#include "3-system/file/readdir.hpp"
#include "3-system/file/readlinkat.hpp"
#include "3-system/file/renameat.hpp"
#include "3-system/file/symlinkat.hpp"
#include "3-system/file/unlinkat.hpp"
#include "3-system/file/write_all.hpp"
#include "3-system/fs/options.hpp"

static auto kind_at(const int dir, const char* const path, const int flags) noexcept -> ca::sys::file_kind {

	auto status = ca::sys::fstatat(dir, path, flags);
	return status ? status->kind() : ca::sys::file_kind::unknown;
}

auto main(void) -> int {
	static_cast<void>(ca::sys::unlinkat(ca::sys::at::cwd, "build/sys_file_at.tmp/sub/moved", 0));
	static_cast<void>(ca::sys::unlinkat(ca::sys::at::cwd, "build/sys_file_at.tmp/sub", ca::sys::at::remove_directory));
	static_cast<void>(ca::sys::unlinkat(ca::sys::at::cwd, "build/sys_file_at.tmp/link", 0));
	static_cast<void>(ca::sys::unlinkat(ca::sys::at::cwd, "build/sys_file_at.tmp/data", 0));
	static_cast<void>(ca::sys::unlinkat(ca::sys::at::cwd, "build/sys_file_at.tmp", ca::sys::at::remove_directory));

	if (!ca::sys::mkdirat(ca::sys::at::cwd, "build/sys_file_at.tmp", 0755U))
		return 1;
	constexpr int directory = ca::fs::access::READ_ONLY | ca::fs::directory_t::OPEN_VALUE | ca::fs::cloexec_t::OPEN_VALUE;
	auto dir = ca::sys::openat(ca::sys::at::cwd, "build/sys_file_at.tmp", directory, 0U);
	if (!dir)
		return 2;
	constexpr int create = ca::fs::access::WRITE_ONLY | ca::fs::create_t::OPEN_VALUE | ca::fs::cloexec_t::OPEN_VALUE;
	auto file = ca::sys::openat(*dir, "data", create, 0644U);
	if (!file || !ca::sys::write_all(*file, ca::sv::str{"1234"}) || !ca::sys::close(*file))
		return 3;
	auto status = ca::sys::fstatat(*dir, "data", 0);
	if (!status || status->size() != 4U || status->kind() != ca::sys::file_kind::regular)
		return 4;

	if (!ca::sys::symlinkat("data", *dir, "link"))
		return 5;
	if (kind_at(*dir, "link", ca::sys::at::symlink_nofollow) != ca::sys::file_kind::symlink
		|| kind_at(*dir, "link", 0) != ca::sys::file_kind::regular)
		return 6;
	char target[16]{};
	auto length = ca::sys::readlinkat(*dir, "link", target);
	if (!length || *length != 4U || target[0] != 'd' || target[3] != 'a')
		return 7;

	if (!ca::sys::mkdirat(*dir, "sub", 0755U) || !ca::sys::linkat(*dir, "link", *dir, "sub/hard", ca::sys::at::symlink_follow))
		return 8;
	auto hard = ca::sys::fstatat(*dir, "sub/hard", ca::sys::at::symlink_nofollow);
	if (!hard || hard->kind() != ca::sys::file_kind::regular || hard->links() != 2U)
		return 9;
	if (!ca::sys::renameat(*dir, "sub/hard", *dir, "sub/moved"))
		return 10;
	auto full = ca::sys::unlinkat(*dir, "sub", ca::sys::at::remove_directory);
	if (full || full.error() != ca::errc::not_empty)
		return 11;

	auto sub = ca::sys::openat(*dir, "sub", directory, 0U);
	if (!sub)
		return 12;
	auto stream = ca::sys::fdopendir(*sub);
	if (!stream)
		return 13;
	auto listed = ca::sys::dirfd(*stream);
	if (!listed || *listed != *sub)
		return 14;
	auto first = ca::sys::readdir(*stream);
	auto second = ca::sys::readdir(*stream);
	if (!first || *first != ca::sv::str{"moved"} || !second || !second->empty())
		return 15;
	if (!ca::sys::unlinkat(*listed, "moved", 0) || !ca::sys::closedir(*stream))
		return 16;

	if (!ca::sys::unlinkat(*dir, "sub", ca::sys::at::remove_directory) || !ca::sys::unlinkat(*dir, "link", 0)
		|| !ca::sys::unlinkat(*dir, "data", 0) || !ca::sys::close(*dir))
		return 17;
	return ca::sys::unlinkat(ca::sys::at::cwd, "build/sys_file_at.tmp", ca::sys::at::remove_directory) ? 0 : 18;
}
