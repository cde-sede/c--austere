// expect: exit 0
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/dynamic_bit_array.hpp"

static_assert(ca::dynamic_bit_array<>::TRIVIALLY_RELOCATABLE && sizeof(ca::dynamic_bit_array<>) == 4U * sizeof(void*));

static auto basics(void) noexcept -> int {

	ca::dynamic_bit_array<> bits;
	if (bits.get(0U) || bits.pop() || bits.any() || !bits.all())
		return 1;
	for (ca::usz i = 0U; i < 200U; ++i)
		if (!bits.append(i % 3U == 0U))
			return 2;
	if (bits.size() != 200U || bits.count() != 67U || !bits[198] || bits[199] || bits.words().size() != 4U)
		return 3;
	if (bits.pop().value_or(true) || bits.pop().value_or(false) != true || bits.size() != 198U)
		return 4;
	if (!bits.resize(300U, true) || bits.count() != 66U + 102U || !bits[198] || bits[197] || !bits[195] || !bits[299])
		return 5;
	if (!bits.resize(64U) || bits.words().size() != 1U || bits.count() != 22U || bits.find_next(64U))
		return 6;
	for (ca::usz i = 0U; i < 64U; ++i)
		if (bits.pop().has_value() != true)
			return 7;
	if (!bits.empty() || bits.words().size() != 0U)
		return 8;
	auto ones = ca::dynamic_bit_array<>::make(70U, true);
	if (!ones || ones->count() != 70U || !ones->all() || ones->words()[1] != 0x3FU)
		return 9;
	auto copy = ones->clone();
	if (!copy || !(*copy == *ones))
		return 10;
	copy->reset(5U);
	*copy ^= *ones;
	if (copy->count() != 1U || copy->find_first().value_or(0U) != 5U)
		return 11;
	ones->flip_all();
	if (!ones->none() || ones->words()[1] != 0U)
		return 12;
	bits.clear();
	return bits.cleanup() && ones->cleanup() && copy->cleanup() ? 0 : 13;
}

static auto arena(void) noexcept -> int {

	auto made = ca::arena::make();
	if (!made)
		return 20;
	ca::arena arena = ca::move(*made);
	{
		auto bits = ca::make_in<ca::dynamic_bit_array<>>(arena, 1000U, false);
		if (!bits || bits->size() != 1000U)
			return 21;
		bits->set(777U);
		if (bits->find_first().value_or(0U) != 777U || !bits->cleanup())
			return 22;
	}
	return arena.cleanup() ? 0 : 23;
}

auto main(void) -> int {

	if (const int failed = basics(); failed != 0)
		return failed;
	return arena();
}
