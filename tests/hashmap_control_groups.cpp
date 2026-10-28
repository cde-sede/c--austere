// expect: exit 0
// link: both
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "5-collections/map/impl/swar_group.hpp"
#include "5-collections/map/impl/vector_group.hpp"

using swar   = ca::impl::swar_group;
using vector = ca::impl::vector_group;

static auto slots(const swar::bits set) noexcept -> ca::u32 {

	ca::u32 lanes = 0U;
	for (swar::bits rest = set; rest != 0U; rest &= rest - 1U)
		lanes |= 1U << swar::first(rest);
	return lanes;
}

static auto slots(const vector::bits set) noexcept -> ca::u32 {
	return set;
}

static auto expected(const ca::u8* const control, const ca::usz width, const ca::u8 tag) noexcept -> ca::u32 {

	ca::u32 lanes = 0U;
	for (ca::usz i = 0U; i < width; ++i)
		if (control[i] == tag)
			lanes |= 1U << i;
	return lanes;
}

static auto free_slots(const ca::u8* const control, const ca::usz width) noexcept -> ca::u32 {

	ca::u32 lanes = 0U;
	for (ca::usz i = 0U; i < width; ++i)
		if ((control[i] & 0x80U) != 0U)
			lanes |= 1U << i;
	return lanes;
}

static ca::u64 state = 0x9E3779B97F4A7C15ULL;

static auto next(void) noexcept -> ca::u64 {

	state ^= state << 13U;
	state ^= state >> 7U;
	state ^= state << 17U;
	return state;
}

auto main(void) -> int {

	ca::u8 control[16];
	for (int round = 0; round < 100000; ++round) {
		for (ca::u8& byte : control) {
			const ca::u64 r = next();
			byte = (r & 3U) == 0U ? swar::EMPTY : (r & 3U) == 1U ? swar::DELETED : static_cast<ca::u8>((r >> 8U) & 0x7FU);
		}
		const ca::u8 tag = static_cast<ca::u8>(next() & 0x7FU);
		const vector v{control};
		if (slots(v.match(tag)) != expected(control, 16U, tag) || slots(v.match_empty()) != expected(control, 16U, vector::EMPTY))
			return 1;
		if (slots(v.match_free()) != free_slots(control, 16U) || slots(v.match_full()) != (free_slots(control, 16U) ^ 0xFFFFU))
			return 2;
		for (ca::usz half = 0U; half < 16U; half += 8U) {
			const swar s{control + half};
			const ca::u32 hits = slots(s.match(tag));
			const ca::u32 exact = expected(control + half, 8U, tag);
			if ((hits & exact) != exact || (hits & ~exact & free_slots(control + half, 8U)) != 0U)
				return 3;
			if (slots(s.match_empty()) != expected(control + half, 8U, swar::EMPTY) || slots(s.match_free()) != free_slots(control + half, 8U))
				return 4;
			if (slots(s.match_full()) != (free_slots(control + half, 8U) ^ 0xFFU))
				return 5;
			const ca::usz lane = static_cast<ca::usz>(next() & 7U);
			if (slots(swar::from(s.match_full(), lane)) != (slots(s.match_full()) & (0xFFU << lane)))
				return 6;
		}
		const ca::usz lane = static_cast<ca::usz>(next() & 15U);
		if (slots(vector::from(v.match_full(), lane)) != (slots(v.match_full()) & (0xFFFFU << lane)))
			return 7;
	}
	return 0;
}
