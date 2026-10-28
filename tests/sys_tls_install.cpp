// expect: exit 0
// link: freestanding
#include "1-primitives/integers/u64.hpp"
#include "3-system/thread/impl/tls.hpp"

alignas(128) static constinit thread_local ca::u64 initialized = 0x1234567890abcdefU;
static constinit thread_local int zeroed = 0;

static char area[1 << 16];

auto main(void) -> int {
	const ca::sys::impl::tls_image image = ca::sys::impl::program_tls;
	if (image._align < 128U || image._size == 0U || image._file_size > image._size || ca::sys::impl::tls_area_size(image) > sizeof(area))
		return 1;
	ca::u64 current = 0U;
	asm("movq %%fs:0, %0" : "=r"(current));
	initialized = 7U;
	zeroed = 9;
	const ca::u64 pointer = ca::sys::impl::install_tls(image, area, 0xabcd00U);
	if (pointer % image._align != 0U || pointer < reinterpret_cast<ca::u64>(area) + image._size)
		return 2;
	if (*reinterpret_cast<ca::u64*>(pointer) != pointer || reinterpret_cast<ca::u64*>(pointer)[5] != 0xabcd00U)
		return 3;
	const ca::u64 at = pointer + (reinterpret_cast<ca::u64>(&initialized) - current);
	const ca::u64 zero_at = pointer + (reinterpret_cast<ca::u64>(&zeroed) - current);
	return *reinterpret_cast<ca::u64*>(at) == 0x1234567890abcdefU && *reinterpret_cast<int*>(zero_at) == 0 ? 0 : 4;
}
