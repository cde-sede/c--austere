// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	if (!a.shrink() || a.capacity() != 0)
		return 1;
	for (int i = 0; i < 5; ++i) {
		if (!a.append(i))
			return 2;
	}
	if (!a.shrink() || a.capacity() < 5 || a[4] != 4)
		return 3;
	if (!a.clear() || !a.shrink() || a.capacity() != 0 || a.data() != nullptr)
		return 4;
	return a.cleanup() ? 0 : 5;
}
