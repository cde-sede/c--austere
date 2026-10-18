// expect: exit 0
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"

static_assert(__is_trivially_copyable(ca::borrowed_slice<int>));

constexpr auto search(void) noexcept -> bool {

	const int values[7] = {1, 2, 3, 1, 2, 3, 4};
	const ca::borrowed_slice all{values};
	const int pair[2] = {2, 3};
	const int missing[2] = {3, 2};
	return all.find(3).value_or(9U) == 2U && all.find_last(3).value_or(9U) == 5U && all.find(pair).value_or(9U) == 1U
		&& all.find_last(pair).value_or(9U) == 4U && !all.find(missing) && all.contains(4) && !all.contains(missing)
		&& all.count(1) == 2U && all.starts_with(all.prefix(3U)) && all.ends_with(all.suffix(2U))
		&& all.find_if([](const int v) { return v > 3; }).value_or(9U) == 6U && all.first().value_or(0) == 1;
}

static_assert(search());

constexpr auto order(void) noexcept -> bool {

	const int a[3] = {1, 2, 3};
	const int b[3] = {1, 2, 4};
	const ca::borrowed_slice<const int> left{a};
	const ca::borrowed_slice<const int> right{b};
	return left.compare(right) < 0 && right.compare(left) > 0 && left.compare(left) == 0 && left.prefix(2U).compare(left) < 0
		&& left == left && !(left == right) && left.prefix(2U) == right.prefix(2U);
}

static_assert(order());

auto main(void) -> int {
	int values[5] = {1, 2, 3, 4, 5};
	ca::borrowed_slice<int> all{values};
	all.reverse();
	if (values[0] != 5 || values[4] != 1)
		return 1;
	all.sub(1U, 3U).fill(0);
	if (values[1] != 0 || values[3] != 0 || values[4] != 1)
		return 2;
	const ca::borrowed_slice<int> shallow = all;
	shallow[0] = 7;
	if (values[0] != 7 || !shallow.last() || shallow.last().value_or(0) != 1)
		return 3;
	const ca::span<int> memory = all;
	const ca::borrowed_slice<const int> back = memory;
	if (back.size() != 5U || back.get(5U) || ca::borrowed_slice<int>{}.first())
		return 4;
	const ca::sv::str text{"key=value"};
	const ca::borrowed_slice<const char> chars = text;
	return chars.find('=').value_or(0U) == 3U && chars.contains(ca::sv::str{"val"}) ? 0 : 5;
}
