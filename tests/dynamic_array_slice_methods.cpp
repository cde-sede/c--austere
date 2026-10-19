// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	for (int i = 0; i < 6; ++i)
		if (!a.append(i % 3))
			return 1;
	if (a.find(2).value_or(9U) != 2U || a.find_last(0).value_or(9U) != 3U || a.count(1) != 2U)
		return 2;
	const int pattern[2] = {2, 0};
	const int pair[2] = {0, 1};
	if (a.find(pattern).value_or(9U) != 2U || !a.starts_with(a.sub(3U, 2U)))
		return 3;
	if (a.count(pattern) != 1U || a.count(pair) != 2U || a.count(a.prefix(0U)) != 7U || a.count(a.view()) != 1U)
		return 7;
	ca::dynamic_array<int> b;
	if (!b.extend(a.prefix(3U)) || !b.extend(a.suffix(3U)) || !(a == b))
		return 4;
	b.suffix(1U).fill(7);
	if (a == b || a.compare(b) >= 0)
		return 5;
	const auto& constant = a;
	const ca::borrowed_slice<const int> view = constant.view();
	return view.size() == 6U && a.first().value_or(9) == 0 && a.cleanup() && b.cleanup() ? 0 : 6;
}
