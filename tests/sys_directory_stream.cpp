// expect: exit 0
// link: both
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/closedir.hpp"
#include "3-system/file/dirfd.hpp"
#include "3-system/file/fdopendir.hpp"
#include "3-system/file/mkdtemp.hpp"
#include "3-system/file/openat.hpp"
#include "3-system/file/opendir.hpp"
#include "3-system/file/readdir.hpp"
#include "3-system/file/rmdir.hpp"
#include "3-system/file/unlinkat.hpp"

static constexpr int COUNT = 700;
static constexpr int CREATE = 0100 | 01 | 02000000;

/* "entry_with_a_long_enough_name_0042": 700 such names fill several getdents64 buffers. */
static auto name_of(const int i, char* const name) -> void {
	const char prefix[] = "entry_with_a_long_enough_name_";
	int at = 0;
	for (; prefix[at] != 0; ++at)
		name[at] = prefix[at];
	name[at++] = static_cast<char>('0' + i / 1000);
	name[at++] = static_cast<char>('0' + i / 100 % 10);
	name[at++] = static_cast<char>('0' + i / 10 % 10);
	name[at++] = static_cast<char>('0' + i % 10);
	name[at] = 0;
}

auto main(void) -> int {
	char path[] = "/tmp/ca_directory_stream_XXXXXX";
	if (!ca::sys::mkdtemp(path))
		return 1;
	auto empty = ca::sys::opendir(path);
	if (!empty)
		return 2;
	auto none = ca::sys::readdir(*empty);
	if (!none || !none->empty() || !ca::sys::closedir(*empty))
		return 3;
	auto listing = ca::sys::opendir(path);
	if (!listing)
		return 4;
	auto dir = ca::sys::dirfd(*listing);
	if (!dir)
		return 5;
	char name[64];
	for (int i = 0; i < COUNT; ++i) {
		name_of(i, name);
		auto fd = ca::sys::openat(*dir, name, CREATE, 0600U);
		if (!fd || !ca::sys::close(*fd))
			return 6;
	}
	bool seen[COUNT] = {};
	int found = 0;
	for (;;) {
		auto entry = ca::sys::readdir(*listing);
		if (!entry)
			return 7;
		if (entry->empty())
			break;
		int index = 0;
		const ca::sv::str text = *entry;
		if (text.size() != 34U)
			return 8;
		for (ca::usz k = 30U; k < 34U; ++k)
			index = index * 10 + (text[k] - '0');
		if (index >= COUNT || seen[index])
			return 9;
		seen[index] = true;
		++found;
	}
	if (found != COUNT)
		return 10;
	for (int i = 0; i < COUNT; ++i) {
		name_of(i, name);
		if (!ca::sys::unlinkat(*dir, name, 0))
			return 11;
	}
	if (!ca::sys::closedir(*listing) || !ca::sys::rmdir(path))
		return 12;
	auto missing = ca::sys::opendir(path);
	if (missing || missing.error() != ca::errc::no_entry)
		return 13;
	auto file = ca::sys::openat(-100, "/proc/self/status", 0, 0U);
	if (!file)
		return 14;
	auto not_directory = ca::sys::fdopendir(*file);
	if (not_directory || not_directory.error() != ca::errc::not_a_directory)
		return 15;
	return ca::sys::close(*file) ? 0 : 16;
}
