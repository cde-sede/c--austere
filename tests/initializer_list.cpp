// expect: exit 0
#include "1-primitives/utility/initializer_list.hpp"
#include "2-vocabulary/borrowed_slice.hpp"

constexpr auto sum(const ca::initializer_list<int> values) noexcept -> int {

	int total = 0;
	for (const int value : values)
		total += value;
	return total;
}

constexpr auto last(const ca::borrowed_slice<const int> values) noexcept -> int {
	return values.last().value_or(-1);
}

static_assert(sum({1, 2, 3}) == 6 && sum({}) == 0);
static_assert(sizeof(ca::initializer_list<int>) == 2U * sizeof(void*));
static_assert(last(ca::initializer_list<int>{4, 5}) == 5);

auto main(void) -> int {

	int total = 0;
	for (const int value : {5, 3, 9})
		total += value;
	const ca::initializer_list<int> held = {7, 8};
	return total == 17 && last(ca::initializer_list<int>{4, 5}) == 5 && held.size() == 2U && *held.begin() == 7 ? 0 : 1;
}
