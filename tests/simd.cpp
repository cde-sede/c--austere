// expect: exit 0
// link: both
#include "ca/integers.hpp"
#include "ca/simd.hpp"
#include "ca/traits.hpp"
#include "1-primitives/config/architecture.hpp"
#include "1-primitives/memory/impl/cpu_features.hpp"

using bytes16 = ca::simd::vec<ca::u8, 16U>;
using bytes32 = ca::simd::vec<ca::u8, 32U>;
using words4  = ca::simd::vec<ca::u32, 4U>;

static_assert(sizeof(bytes16) == 16U && sizeof(bytes32) == 32U && sizeof(words4) == 16U);
static_assert(ca::simd::lanes<bytes16> == 16U && ca::simd::lanes<words4> == 4U);
static_assert(ca::is_same<ca::simd::lane_of<words4>, ca::u32>);
static_assert(ca::is_same<ca::simd::mask<16U>::word, ca::u32> && ca::is_same<ca::simd::mask<64U>::word, ca::u64>);

static_assert([] {
	const ca::simd::mask<16U> m{0x8421U};
	ca::usz seen = 0U;
	ca::usz sum = 0U;
	for (const ca::usz lane : m) {
		++seen;
		sum += lane;
	}
	return seen == 4U && sum == 0U + 5U + 10U + 15U && m.count() == 4U && m.first() == 0U && m.last() == 15U
	    && m.test(5U) && !m.test(6U) && !m.test(16U) && m.below(6U).bits() == 0x21U && m.from(6U).bits() == 0x8400U
	    && m.below(16U) == m && m.from(16U).none() && (~m).count() == 12U && ca::simd::mask<16U>::full().all()
	    && ca::simd::mask<16U>{}.first() >= 16U && ca::simd::mask<16U>{}.last() >= 16U
	    && (m & ca::simd::mask<16U>{0x0021U}).bits() == 0x21U && (m | ca::simd::mask<16U>{0x0002U}).count() == 5U
	    && (m ^ m).none() && ca::simd::mask<64U>::full().count() == 64U && ca::simd::mask<32U>::full().all();
}());

static_assert([] {
	bytes16 v{};
	ca::simd::splat(7, v);
	const bytes16 w{0U, 0U, 0U, 7U, 0U, 0U, 0U, 0U, 0U, 7U, 0U, 0U, 0U, 0U, 0U, 0U};
	const bytes16 all = v;
	return ca::simd::mask_of(v == w).bits() == 0x208U && ca::simd::mask_of(v == all).all();
}());

static volatile int seed = 0x5a;

static auto check16(const ca::u8* const data) noexcept -> int {

	bytes16 v;
	bytes16 needle;
	ca::simd::load(data, v);
	ca::simd::splat(seed, needle);
	if (ca::simd::mask_of(v == needle).bits() != 0x8001U)
		return 1;
	bytes16 aligned;
	ca::simd::load_aligned(data, aligned);
	if (ca::simd::mask_of(aligned == v).bits() != 0xffffU)
		return 2;
	ca::u8 out[32] = {};
	ca::simd::store(out + 1, v);
	if (out[0] != 0U || out[1] != 0x5aU || out[16] != 0x5aU || out[17] != 0U)
		return 3;
	alignas(16) ca::u8 aligned_out[16] = {};
	ca::simd::store_aligned(aligned_out, needle);
	for (const ca::u8 byte : aligned_out)
		if (byte != 0x5aU)
			return 4;
	words4 a{1U, 2U, 3U, 4U};
	words4 b;
	ca::simd::splat(3U, b);
	if (ca::simd::mask_of(a < b).bits() != 0x3U || ca::simd::mask_of(a == b).first() != 2U)
		return 5;
#if ca_architecture(x86_64)
	bytes16 peeked;
	ca::simd::peek(data, peeked);
	bytes16 peeked_aligned;
	ca::simd::peek_aligned(data, peeked_aligned);
	if (ca::simd::mask_of(peeked == v).bits() != 0xffffU || ca::simd::mask_of(peeked_aligned == v).bits() != 0xffffU)
		return 6;
#endif
	return 0;
}

#if ca_architecture(x86_64)

[[gnu::target("avx2"), gnu::noinline]] static auto check32(const ca::u8* const data) noexcept -> int {

	bytes32 v;
	bytes32 needle;
	ca::simd::peek_aligned(data, v);
	ca::simd::splat(seed, needle);
	if (ca::simd::mask_of(v == needle).bits() != 0x80008001U)
		return 11;
	bytes32 loose;
	ca::simd::peek(data + 1, loose);
	if (ca::simd::mask_of(loose == needle).bits() != 0x40004000U)
		return 12;
	ca::simd::mask<32U> lanes = ca::simd::mask_of(v == needle);
	ca::usz sum = 0U;
	for (const ca::usz lane : lanes)
		sum += lane;
	return sum == 0U + 15U + 31U ? 0 : 13;
}

#endif

auto main(void) -> int {

	alignas(32) ca::u8 data[64] = {};
	data[0] = 0x5aU;
	data[15] = 0x5aU;
	data[31] = 0x5aU;
	if (const int r = check16(data); r != 0)
		return r;
#if ca_architecture(x86_64)
	if (ca::impl::read_cpu_features()._avx2)
		return check32(data);
#endif
	return 0;
}
