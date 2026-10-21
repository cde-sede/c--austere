// expect: exit 0
#include "1-primitives/hash/hash_bytes.hpp"
#include "1-primitives/integers/i64.hpp"
#include "2-vocabulary/sv/u16.hpp"
#include "5-collections/array/array.hpp"
#include "5-collections/map/hash.hpp"
#include "5-collections/string/str.hpp"

enum class color : ca::u8 { red = 3 };

struct point {
	int x;
	int y;
	auto hash(void) const noexcept -> ca::u64 {
		return ca::hash_word(static_cast<ca::u64>(x) * 31U + static_cast<ca::u64>(y));
	}
};

static_assert(ca::is_hashable<int> && ca::is_hashable<ca::str> && ca::is_hashable<ca::sv::u16> && ca::is_hashable<point>);
static_assert(!ca::is_hashable<double> && !ca::is_hashable<ca::array<int, 2>>);

auto main(void) -> int {

	if (ca::hash(5) != ca::hash(5U) || ca::hash(-1) != ca::hash(static_cast<ca::i64>(-1)) || ca::hash(5) == ca::hash(6))
		return 1;
	if (ca::hash(color::red) != ca::hash(3))
		return 2;
	auto text = ca::str::make("hello, hash");
	if (!text || ca::hash(*text) != ca::hash(ca::sv::str{"hello, hash"}) || ca::hash(*text) == ca::hash(ca::sv::str{"hello, hasH"}))
		return 3;
	char buffer[200];
	for (ca::usz i = 0U; i < sizeof(buffer); ++i)
		buffer[i] = static_cast<char>(i);
	ca::u64 previous = 0U;
	for (ca::usz size = 0U; size <= sizeof(buffer); ++size) {
		const ca::u64 h = ca::hash_bytes(buffer, size);
		if (h == previous || h != ca::hash_bytes(buffer, size))
			return 4;
		previous = h;
		if (size != 0U) {
			buffer[size - 1U] ^= 1;
			const bool changed = ca::hash_bytes(buffer, size) != h;
			buffer[size - 1U] ^= 1;
			if (!changed)
				return 5;
		}
	}
	if (ca::hash_bytes(buffer, 16U, 1U) == ca::hash_bytes(buffer, 16U, 2U))
		return 6;
	if (ca::hash(point{1, 2}) == ca::hash(point{2, 1}))
		return 7;
	return text->cleanup() ? 0 : 8;
}
