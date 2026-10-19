// expect: exit 0
// link: both
#include "1-primitives/bit/popcount.hpp"
#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"

static_assert(ca::popcount(ca::u8{0xffU}) == 8);
static_assert(ca::popcount(ca::u64{0x8000000000000001ULL}) == 2);
static_assert(ca::popcount(ca::u32{0}) == 0);

static volatile ca::u64 seed = 0x9E3779B97F4A7C15ULL;

auto main(void) -> int {
	ca::u64 value = seed;
	for (int i = 0; i < 100000; ++i) {
		value ^= value << 13U;
		value ^= value >> 7U;
		value ^= value << 17U;
		int expected = 0;
		for (ca::u64 rest = value; rest != 0U; rest &= rest - 1U)
			++expected;
		if (ca::popcount(value) != expected)
			return 1;
		if (ca::popcount(static_cast<ca::u16>(value)) != ca::popcount(static_cast<ca::u64>(static_cast<ca::u16>(value))))
			return 2;
	}
	return 0;
}
