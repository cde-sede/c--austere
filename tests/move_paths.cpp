// expect: exit 0
#include "3-system/memory/move_paths.hpp"

static constexpr ca::usz SPAN = 4096U;

static char work[SPAN];
static char expected[SPAN];

using path = auto (*)(void*, const void*, ca::usz) noexcept -> ca::result<void>;

static auto any_sse2(void* const to, const void* const from, const ca::usz count) noexcept -> ca::result<void> {
	ca::move_paths::any(to, from, count);
	return ca::result<void>{};
}

static auto any_avx2(void* const to, const void* const from, const ca::usz count) noexcept -> ca::result<void> {
	ca::move_paths::avx2::any(to, from, count);
	return ca::result<void>{};
}

static auto pattern(const ca::usz i, const unsigned seed) -> char {
	return static_cast<char>((i * 131U + seed) ^ (i >> 8U));
}

static auto reset(const unsigned seed) -> void {
	for (ca::usz i = 0; i < SPAN; ++i)
		work[i] = expected[i] = pattern(i, seed);
}

static auto same(void) -> bool {
	for (ca::usz i = 0; i < SPAN; ++i)
		if (work[i] != expected[i])
			return false;
	return true;
}

static auto check(const path move, const ca::usz count, const ca::usz from, const ca::usz to) -> bool {
	reset(static_cast<unsigned>(count + from * 3U + to));
	for (ca::usz i = 0; i < count; ++i)
		expected[to + i] = pattern(from + i, static_cast<unsigned>(count + from * 3U + to));
	return move(work + to, work + from, count) && same();
}

static const ca::usz offsets[] = {1U, 3U, 16U, 33U};

static auto disjoint(const path move, const ca::usz low, const ca::usz high) -> bool {
	for (ca::usz count = low; count <= high; ++count)
		for (const ca::usz at : offsets)
			if (!check(move, count, at, SPAN / 2U + at))
				return false;
	return true;
}

static auto overlapping(const path move, const ca::usz low, const ca::usz high, const bool down, const bool up) -> bool {
	for (ca::usz count = low; count <= high; ++count)
		for (const ca::usz delta : offsets) {
			if (down && !check(move, count, 64U + delta, 64U))
				return false;
			if (up && !check(move, count, 64U, 64U + delta))
				return false;
		}
	return true;
}

static auto small(const path move, const ca::usz low, const ca::usz high) -> bool {
	return disjoint(move, low, high) && overlapping(move, low, high, true, true);
}

static auto large(const path forward, const path backward, const path repeat, const path stream, const path pick, const path any, const ca::usz above) -> bool {
	const ca::usz low = above + 1U;
	const ca::usz high = 1500U;
	return disjoint(forward, low, high) && overlapping(forward, low, high, true, false) && disjoint(backward, low, high)
		&& overlapping(backward, low, high, false, true) && disjoint(repeat, low, high) && disjoint(stream, low, high)
		&& disjoint(pick, low, high) && overlapping(pick, low, high, true, true) && disjoint(any, 0U, high)
		&& overlapping(any, 0U, high, true, true);
}

static auto has_avx2(void) -> bool {
	unsigned a = 0U, b = 0U, c = 0U, d = 0U;
	asm("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0U), "c"(0U));
	if (a < 7U)
		return false;
	asm("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(1U), "c"(0U));
	if ((c >> 27U & 1U) == 0U || (c >> 28U & 1U) == 0U)
		return false;
	unsigned low = 0U, high = 0U;
	asm("xgetbv" : "=a"(low), "=d"(high) : "c"(0U));
	if ((low & 6U) != 6U)
		return false;
	asm("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(7U), "c"(0U));
	return (b >> 5U & 1U) != 0U;
}

auto main(void) -> int {
	if (!small(&ca::move_paths::bytes_0_to_15, 0U, 15U) || !small(&ca::move_paths::bytes_16_to_32, 16U, 32U))
		return 1;
	if (!small(&ca::move_paths::bytes_32_to_64, 32U, 64U) || !small(&ca::move_paths::bytes_64_to_128, 64U, 128U))
		return 2;
	if (!small(&ca::move_paths::bytes_0_to_64, 0U, 64U))
		return 3;
	if (!large(&ca::move_paths::forward, &ca::move_paths::backward, &ca::move_paths::repeat, &ca::move_paths::stream,
			&ca::move_paths::large, &any_sse2, 128U))
		return 4;
	if (!has_avx2())
		return 0;
	if (!small(&ca::move_paths::avx2::bytes_32_to_64, 32U, 64U) || !small(&ca::move_paths::avx2::bytes_64_to_128, 64U, 128U)
		|| !small(&ca::move_paths::avx2::bytes_128_to_256, 128U, 256U))
		return 5;
	if (!large(&ca::move_paths::avx2::forward, &ca::move_paths::avx2::backward, &ca::move_paths::avx2::repeat,
			&ca::move_paths::avx2::stream, &ca::move_paths::avx2::large, &any_avx2, 256U))
		return 6;
	return 0;
}
