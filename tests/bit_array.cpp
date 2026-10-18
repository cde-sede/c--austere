// expect: exit 0
#include "1-primitives/traits/is_trivially_copyable.hpp"
#include "5-collections/array/bit_array.hpp"

static_assert(sizeof(ca::bit_array<100>) == 16U && sizeof(ca::bit_array<64>) == 8U && ca::is_trivially_copyable<ca::bit_array<9>>);

constexpr auto build(void) noexcept -> bool {

	ca::bit_array<130> bits;
	if (bits.any() || bits.count() != 0U || bits.find_first() || bits.find_first_unset().value_or(9U) != 0U)
		return false;
	bits.set(0U);
	bits.set(64U);
	bits.set(129U);
	bits.flip(3U);
	bits.set(3U, false);
	if (bits.count() != 3U || !bits[64] || bits[3] || bits.get(130U) || bits.find_next(1U).value_or(0U) != 64U || bits.find_next(130U))
		return false;
	ca::usz sum = 0U;
	ca::usz seen = 0U;
	for (const ca::usz index : bits.ones()) {
		sum += index;
		++seen;
	}
	if (seen != 3U || sum != 193U)
		return false;
	ca::bit_array<130> other;
	other.set_all();
	if (!other.all() || other.count() != 130U || other.find_first_unset() || other.find_next_unset(5U))
		return false;
	other.flip_all();
	if (!other.none() || other.count() != 0U)
		return false;
	other.set(64U);
	other.set(1U);
	other &= bits;
	if (other.count() != 1U || !other[64])
		return false;
	other |= bits;
	if (!(other == bits))
		return false;
	other ^= bits;
	bits.reset_all();
	return other.none() && other == bits;
}

static_assert(build());

auto main(void) -> int {

	if (!build())
		return 1;
	ca::bit_array<70> bits;
	bits.set_all();
	bits.reset(69U);
	bits.reset(0U);
	return bits.find_first().value_or(0U) == 1U && bits.find_first_unset().value_or(1U) == 0U && bits.find_next_unset(1U).value_or(0U) == 69U && bits.count() == 68U ? 0 : 2;
}
