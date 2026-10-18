// expect: exit 0
#include "3-system/error/errc.hpp"
#include "3-system/memory/protection.hpp"
#include "3-system/syscall/syscalls.hpp"

static_assert(ca::syscall::impl::inline_entry);

constexpr int map_private   = 0x02;
constexpr int map_anonymous = 0x20;

auto main(void) -> int {
	constexpr int rw = ca::sys::protection::read | ca::sys::protection::write;
	auto block = ca::syscall::mmap(nullptr, 4096U, rw, map_private | map_anonymous, -1, 0);
	if (!block)
		return 1;
	char* const bytes = reinterpret_cast<char*>(*block);
	bytes[4095] = 7;
	if (bytes[0] != 0 || bytes[4095] != 7 || !ca::syscall::munmap(bytes, 4096U))
		return 2;

	auto flags_lost = ca::syscall::mmap(nullptr, 4096U, rw, map_anonymous, -1, 0);
	if (flags_lost || flags_lost.error() != ca::errc::invalid_argument)
		return 3;
	auto fd_seen = ca::syscall::mmap(nullptr, 4096U, rw, map_private, -1, 0);
	if (fd_seen || fd_seen.error() != ca::errc::bad_descriptor)
		return 4;
	auto offset_seen = ca::syscall::mmap(nullptr, 4096U, rw, map_private | map_anonymous, -1, 1);
	if (offset_seen || offset_seen.error() != ca::errc::invalid_argument)
		return 5;
	return 0;
}
