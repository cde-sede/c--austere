// expect: exit 0
#include "3-system/error/errc.hpp"
#include "3-system/memory/mmap_anonymous.hpp"
#include "3-system/memory/mprotect.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/page_size.hpp"
#include "3-system/memory/protection.hpp"

auto main(void) -> int {
	const ca::usz page = ca::sys::page_size();
	if (page < 4096U || (page & (page - 1U)) != 0U)
		return 1;
	auto block = ca::sys::mmap_anonymous(2U * page, ca::sys::protection::read | ca::sys::protection::write);
	if (!block)
		return 2;
	const ca::span<ca::byte> bytes = *block;
	if (bytes.size() != 2U * page || bytes[page] != 0)
		return 3;
	bytes[page] = 42;
	if (!ca::sys::mprotect(bytes, ca::sys::protection::read) || bytes[page] != 42)
		return 4;
	auto unaligned = ca::sys::mprotect(bytes.sub(1U, page), ca::sys::protection::read);
	if (unaligned || unaligned.error() != ca::errc::invalid_argument)
		return 5;
	return ca::sys::munmap(bytes) ? 0 : 6;
}
