// expect: exit 0
// link: both
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/ftruncate.hpp"
#include "3-system/file/open.hpp"
#include "3-system/file/pread.hpp"
#include "3-system/file/unlink.hpp"
#include "3-system/fs/options.hpp"
#include "3-system/memory/advice.hpp"
#include "3-system/memory/madvise.hpp"
#include "3-system/memory/mmap_file.hpp"
#include "3-system/memory/msync.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/page_size.hpp"
#include "3-system/memory/protection.hpp"

auto main(void) -> int {
	constexpr int flags = ca::fs::access::READ_WRITE | ca::fs::create_t::OPEN_VALUE | ca::fs::truncate_t::OPEN_VALUE
		| ca::fs::cloexec_t::OPEN_VALUE;
	auto fd = ca::sys::open("build/sys_mmap_file.tmp", flags, 0600U);
	const ca::usz page = ca::sys::page_size();
	if (!fd || !ca::sys::ftruncate(*fd, 2U * page))
		return 1;
	constexpr int rw = ca::sys::protection::read | ca::sys::protection::write;

	auto shared = ca::sys::mmap_file(*fd, page, page, rw, ca::sys::sharing::shared);
	if (!shared)
		return 2;
	const ca::span<ca::byte> bytes = *shared;
	if (bytes[0] != 0 || !ca::sys::madvise(bytes, ca::sys::advice::sequential))
		return 3;
	bytes[0] = 'S';
	bytes[page - 1U] = 'E';
	if (!ca::sys::msync(bytes))
		return 4;
	char got[2]{};
	auto read = ca::sys::pread(*fd, ca::span{got, 1U}, page);
	auto last = ca::sys::pread(*fd, ca::span{got + 1, 1U}, 2U * page - 1U);
	if (!read || !last || got[0] != 'S' || got[1] != 'E')
		return 5;

	auto copy = ca::sys::mmap_file(*fd, page, page, rw, ca::sys::sharing::copy_on_write);
	if (!copy)
		return 6;
	const ca::span<ca::byte> mine = *copy;
	if (mine[0] != 'S')
		return 7;
	mine[0] = 'C';
	auto still = ca::sys::pread(*fd, ca::span{got, 1U}, page);
	if (!still || got[0] != 'S' || bytes[0] != 'S')
		return 8;

	auto unaligned = ca::sys::mmap_file(*fd, 1U, page, rw, ca::sys::sharing::shared);
	if (unaligned || unaligned.error() != ca::errc::invalid_argument)
		return 9;
	if (!ca::sys::munmap(mine) || !ca::sys::munmap(bytes) || !ca::sys::close(*fd))
		return 10;
	return ca::sys::unlink("build/sys_mmap_file.tmp") ? 0 : 11;
}
