// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	const int values[5] = {1, 2, 3, 4, 5};
	for (int round = 0; round < 100; ++round)
		if (!a.extend(values))
			return 1;
	if (a.size() != 500U || a[0] != 1 || a[499] != 5 || a.capacity() < 500U)
		return 2;
	if (!a.extend(ca::borrowed_slice{values}.prefix(0U)) || a.size() != 500U)
		return 3;
	return a.destroy() ? 0 : 4;
}
