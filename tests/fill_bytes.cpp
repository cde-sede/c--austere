// expect: exit 0
#include "1-primitives/memory/fill_bytes.hpp"
#include "1-primitives/memory/impl/cpu_features.hpp"
#include "1-primitives/memory/impl/fill_bytes.hpp"

extern "C" auto memset(void* to, int value, ca::usz count) noexcept -> void*;

static constexpr ca::usz LARGE = (ca::usz{1} << 20U) + 4096U;
static constexpr ca::usz GUARD = 64U;

static char work[LARGE + 2U * GUARD];

static auto (*volatile memset_c)(void*, int, ca::usz) noexcept -> void* = &memset;

static const ca::usz offsets[] = {0U, 1U, 7U, 15U, 16U, 31U, 33U, 63U};
static const int values[] = {0, 0x5a, 0xff, 0x80, 300, -1};
static const ca::usz large[] = {1023U, 2047U, 2048U, 2049U, 4095U, 4097U, 65536U + 3U, ca::usz{1} << 20U};

static auto check(const ca::impl::fill_bytes_function fill, const ca::usz count, const ca::usz offset, const int value) -> bool {
	const ca::usz span = count + offset + GUARD;
	for (ca::usz i = 0; i < span + GUARD; ++i)
		work[i] = static_cast<char>(i * 7U + 1U);
	if (fill(work + GUARD + offset, value, count) != work + GUARD + offset)
		return false;
	for (ca::usz i = 0; i < span + GUARD; ++i) {
		const bool inside = i >= GUARD + offset && i < GUARD + offset + count;
		const char expected = inside ? static_cast<char>(value) : static_cast<char>(i * 7U + 1U);
		if (work[i] != expected)
			return false;
	}
	return true;
}

static auto all(const ca::impl::fill_bytes_function fill) -> bool {
	for (const ca::usz offset : offsets) {
		for (const int value : values) {
			for (ca::usz count = 0U; count <= 1100U; ++count)
				if (!check(fill, count, offset, value))
					return false;
			for (const ca::usz count : large)
				if (!check(fill, count, offset, value))
					return false;
		}
	}
	return true;
}

static auto fill_builtin(void* const to, const int value, const ca::usz count) noexcept -> void* {
	ca::fill_bytes(to, static_cast<ca::u8>(value), count);
	return to;
}

auto main(void) -> int {
	if (!all(memset_c))
		return 1;
	if (!all(&fill_builtin))
		return 2;
	if (!all(ca::impl::fill_bytes_variant.load()))
		return 3;
	if (!all(&ca::impl::fill_bytes_sse2))
		return 4;
	if (ca::impl::read_cpu_features()._avx2 && !all(&ca::impl::fill_bytes_avx2))
		return 5;
	ca::fill_bytes(nullptr, 0U, 0U);
	return memset_c(nullptr, 0, 0U) == nullptr ? 0 : 6;
}
