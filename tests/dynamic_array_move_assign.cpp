// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	ca::dynamic_array<int> b;
	if (!a.append(1) || !b.append(2))
		return 1;
	a = ca::move(b);
	if (a.size() != 1 || a[0] != 2 || b.size() != 0)
		return 2;

	ca::dynamic_array<ca::dynamic_array<int>> outer;
	ca::dynamic_array<ca::dynamic_array<int>> other;
	if (!outer.append(ca::move(a)) || !other.emplace())
		return 3;
	if (!outer.clear())
		return 4;
	outer = ca::move(other);
	return outer.size() == 1 && outer.destroy() ? 0 : 5;
}
