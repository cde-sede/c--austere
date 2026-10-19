// expect: exit 0
// ca::sys::realpath (own resolution on linux x86-64) against glibc's realpath: relative and
// absolute symlinks, chains, a loop, ".." through a symlink, a trailing slash on a file, and
// missing components; same text, or the same error.
#include "1-primitives/memory/string_length.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/chdir.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/mkdir.hpp"
#include "3-system/file/mkdtemp.hpp"
#include "3-system/file/open.hpp"
#include "3-system/file/realpath.hpp"
#include "3-system/file/symlink.hpp"
#include "3-system/memory/free.hpp"

extern "C" auto realpath(const char* path, char* resolved) noexcept -> char*;
extern "C" auto free(void* block) noexcept -> void;
extern "C" auto strcmp(const char* a, const char* b) noexcept -> int;
extern "C" auto __errno_location(void) noexcept -> int*;

static auto agrees(const char* const path) -> bool {
	char* const libc = realpath(path, nullptr);
	const int libc_error = libc == nullptr ? *__errno_location() : 0;
	auto ours = ca::sys::realpath(path);
	bool same;
	if (libc == nullptr)
		same = !ours && ours.error().code() == libc_error;
	else
		same = ours && strcmp(*ours, libc) == 0;
	if (ours)
		ca::sys::free(*ours);
	free(libc);
	return same;
}

auto main(void) -> int {
	char dir[] = "/tmp/ca_realpath_XXXXXX";
	if (!ca::sys::mkdtemp(dir) || !ca::sys::chdir(dir))
		return 1;
	if (!ca::sys::mkdir("a", 0700U) || !ca::sys::mkdir("a/b", 0700U))
		return 2;
	auto file = ca::sys::open("a/b/file", 0100 | 01, 0600U);
	if (!file || !ca::sys::close(*file))
		return 3;
	if (!ca::sys::symlink("a/b", "relative") || !ca::sys::symlink(dir, "absolute") || !ca::sys::symlink("relative", "chain"))
		return 4;
	if (!ca::sys::symlink("loop", "loop") || !ca::sys::symlink("../b", "a/b/up") || !ca::sys::symlink("missing", "dangling"))
		return 5;
	if (!ca::sys::symlink("a/b/file", "to_file") || !ca::sys::symlink("to_file", "to_link"))
		return 8;
	const char* const paths[] = {
		".", "..", "/", "//", "a", "a/", "a/./b/", "a/b/..", "a/b/../..", "relative", "relative/file", "chain/file",
		"absolute/a/b/file", "a/b/up", "a/b/up/file", "relative/../b", "loop", "dangling", "a/b/file/", "a/b/file/x",
		"nothing", "nothing/..", "a/../../../..", "", "relative/./../b/./file", "///a//b///file", "to_file", "to_link",
		"to_file/", "to_link/x",
	};
	for (const char* const path : paths) {
		if (!agrees(path))
			return 6;
	}
	if (!agrees(dir))
		return 7;
	return 0;
}
