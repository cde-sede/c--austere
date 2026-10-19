// expect: exit 0
#include "1-primitives/memory/compare_bytes.hpp"
#include "1-primitives/memory/equal_units.hpp"
#include "1-primitives/memory/impl/compare_bytes.hpp"
#include "1-primitives/memory/impl/cpu_features.hpp"

extern "C" auto memcmp(const void* a, const void* b, ca::usz count) noexcept -> int;
extern "C" auto bcmp(const void* a, const void* b, ca::usz count) noexcept -> int;

static constexpr ca::usz LARGE = (ca::usz{1} << 20U) + 4096U;

static char left[LARGE + 128U];
static char right[LARGE + 128U];

static auto (*volatile memcmp_c)(const void*, const void*, ca::usz) noexcept -> int = &memcmp;
static auto (*volatile bcmp_c)(const void*, const void*, ca::usz) noexcept -> int = &bcmp;

static const ca::usz offsets[][2] = {{0U, 0U}, {1U, 0U}, {0U, 3U}, {15U, 33U}, {32U, 31U}};
static const ca::usz large[] = {1023U, 4095U, 4097U, 65536U + 3U, ca::usz{1} << 20U};
static const unsigned char pairs[][2] = {{1U, 2U}, {0x7fU, 0x80U}, {0x00U, 0xffU}, {0xfeU, 0xffU}};

static auto sign(const int x) -> int {
	return (x > 0) - (x < 0);
}

static auto pattern(const ca::usz i) -> char {
	return static_cast<char>((i * 131U) ^ (i >> 8U));
}

/* a and b equal over count (and differ just past it), then a difference at each of positions,
   the last byte differing the other way: the first difference decides. */
static auto check(const ca::impl::compare_bytes_function compare, const bool locate, const ca::usz count, const ca::usz* const offset) -> bool {
	char* const a = left + offset[0];
	char* const b = right + offset[1];
	for (ca::usz i = 0; i < count; ++i) {
		a[i] = pattern(i);
		b[i] = pattern(i);
	}
	a[count] = 1;
	b[count] = 2;
	if (compare(a, b, count) != 0)
		return false;
	const ca::usz positions[] = {0U, count / 2U, count - 1U, count > 17U ? count - 17U : 0U};
	const ca::usz every = count <= 140U ? count : 0U;
	for (ca::usz k = 0; k < every + 4U; ++k) {
		const ca::usz at = k < every ? k : positions[k - every];
		if (count == 0U)
			break;
		for (const auto& pair : pairs) {
			for (int side = 0; side < 2; ++side) {
				const ca::usz last = count - 1U;
				a[at] = static_cast<char>(pair[side]);
				b[at] = static_cast<char>(pair[1 - side]);
				if (last > at) {
					a[last] = static_cast<char>(pair[1 - side]);
					b[last] = static_cast<char>(pair[side]);
				}
				const int got = compare(a, b, count);
				a[at] = pattern(at);
				b[at] = pattern(at);
				a[last] = pattern(last);
				b[last] = pattern(last);
				if (locate ? sign(got) != (side == 0 ? -1 : 1) : got == 0)
					return false;
			}
		}
	}
	return true;
}

static auto all(const ca::impl::compare_bytes_function compare, const bool locate) -> bool {
	for (const auto& offset : offsets) {
		for (ca::usz count = 0U; count <= 700U; ++count)
			if (!check(compare, locate, count, offset))
				return false;
		for (const ca::usz count : large)
			if (!check(compare, locate, count, offset))
				return false;
	}
	return true;
}

static auto compare_builtin(const void* const a, const void* const b, const ca::usz count) noexcept -> int {
	return ca::compare_bytes(static_cast<const char*>(a), static_cast<const char*>(b), count);
}

static auto differ_builtin(const void* const a, const void* const b, const ca::usz count) noexcept -> int {
	return !ca::equal_units(static_cast<const char*>(a), static_cast<const char*>(b), count);
}

static_assert(ca::compare_bytes("abc", "abd", 3U) < 0);
static_assert(ca::compare_bytes("\x80", "\x7f", 1U) > 0);

auto main(void) -> int {
	if (!all(memcmp_c, true) || !all(&compare_builtin, true))
		return 1;
	if (!all(bcmp_c, false) || !all(&differ_builtin, false))
		return 2;
	if (!all(&ca::impl::compare_bytes_sse2, true) || !all(&ca::impl::differ_bytes_sse2, false))
		return 3;
	const ca::impl::cpu_features cpu = ca::impl::read_cpu_features();
	if (cpu._avx2 && cpu._bmi2 && (!all(&ca::impl::compare_bytes_avx2, true) || !all(&ca::impl::differ_bytes_avx2, false)))
		return 4;
	return memcmp_c(nullptr, nullptr, 0U) == 0 && bcmp_c(nullptr, nullptr, 0U) == 0 ? 0 : 5;
}
