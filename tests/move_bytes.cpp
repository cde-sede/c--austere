// expect: exit 0
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/move_bytes.hpp"

static constexpr ca::usz LARGE = (ca::usz{7} << 20U) + 4096U;

static char source[LARGE];
static char work[LARGE];
static char expected[LARGE];

static auto pattern(const ca::usz i, const unsigned seed) -> char {
	return static_cast<char>((i * 131U + seed) ^ (i >> 8U));
}

static auto fill(char* const to, const ca::usz count, const unsigned seed) -> void {
	for (ca::usz i = 0; i < count; ++i)
		to[i] = pattern(i, seed);
}

static auto same(const char* const a, const char* const b, const ca::usz count) -> bool {
	for (ca::usz i = 0; i < count; ++i)
		if (a[i] != b[i])
			return false;
	return true;
}

static auto copy_case(const ca::usz count, const ca::usz from, const ca::usz to) -> bool {
	const ca::usz span = count + 128U;
	fill(work, span, 3U);
	fill(expected, span, 3U);
	for (ca::usz i = 0; i < count; ++i)
		expected[to + i] = source[from + i];
	ca::copy_bytes(work + to, source + from, count);
	return same(work, expected, span);
}

static auto move_case(const ca::usz count, const ca::usz from, const ca::usz to) -> bool {
	const ca::usz span = count + (from > to ? from : to) + 64U;
	fill(work, span, 5U);
	fill(expected, span, 5U);
	for (ca::usz i = 0; i < count; ++i)
		expected[to + i] = pattern(from + i, 5U);
	ca::move_bytes(work + to, work + from, count);
	return same(work, expected, span);
}

static const ca::usz offsets[] = {0U, 1U, 3U, 15U, 16U, 31U, 32U, 33U, 63U};
static const ca::usz deltas[] = {1U, 2U, 7U, 16U, 31U, 32U, 33U, 100U, 255U, 256U, 257U, 1000U, 4096U};
static const ca::usz shifts[] = {0U, 1U, 33U};
static const ca::usz aliases[] = {0U, 1U, 64U, 200U, 255U};
static const ca::usz large_sizes[] = {
	4095U, 4096U, 8191U, 8192U, 8193U, 65537U, (ca::usz{1} << 20U) + 3U, (ca::usz{6} << 20U) - 1U,
	ca::usz{6} << 20U, (ca::usz{7} << 20U) - 4097U
};

auto main(void) -> int {
	fill(source, LARGE, 1U);
	for (ca::usz count = 0; count <= 1100U; ++count)
		for (const ca::usz from : offsets)
			for (const ca::usz to : offsets)
				if (!copy_case(count, from, to))
					return 1;
	for (ca::usz count = 0; count <= 1100U; count += count < 300U ? 1U : 37U)
		for (const ca::usz delta : deltas)
			for (const ca::usz base : offsets) {
				if (!move_case(count, base + delta, base))
					return 2;
				if (!move_case(count, base, base + delta))
					return 3;
			}
	for (const ca::usz count : large_sizes)
		for (const ca::usz shift : shifts) {
			if (!copy_case(count, shift, 64U - shift))
				return 4;
			if (!move_case(count, shift + 100U, shift))
				return 5;
			if (!move_case(count, shift, shift + 100U))
				return 6;
			if (!move_case(count, shift, shift + 4096U - 64U))
				return 7;
		}
	for (ca::usz count = 257U; count <= 2100U; ++count)
		for (const ca::usz alias : aliases)
			for (const ca::usz shift : shifts) {
				if (!move_case(count, shift, shift + 8192U + alias))
					return 10;
				if (!move_case(count, shift + 8192U + alias, shift))
					return 11;
			}
	for (const ca::usz count : large_sizes)
		for (const ca::usz alias : aliases)
			if (!move_case(count / 4U, 1U, 1U + (ca::usz{2} << 20U) + alias))
				return 12;
	for (ca::usz count = 0; count < 64U; ++count)
		if (!move_case(count, 0U, 0U))
			return 8;
	char small[16];
	ca::copy_bytes(small, source, 16U);
	if (!same(small, source, 16U))
		return 9;
	return 0;
}
