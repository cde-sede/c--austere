// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<ca::dynamic_array<int>> outer;
	ca::dynamic_array<int> inner;
	if (!inner.append(1))
		return 1;
	if (!outer.append(ca::move(inner)))
		return 2;
	if (inner.size() != 0 || outer[0][0] != 1)
		return 3;
	return outer.cleanup() ? 0 : 4;
}
