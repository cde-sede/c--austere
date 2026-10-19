// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	if (a.first() || a.last())
		return 1;
	if (!a.append(1) || !a.append(2) || !a.append(3))
		return 2;
	auto f = a.first();
	auto l = a.last();
	if (!f || !l || *f != 1 || *l != 3)
		return 3;
	*l = 7;
	return a[2] == 7 && a.cleanup() ? 0 : 4;
}
