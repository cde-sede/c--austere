// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	for (int i = 0; i < 100; ++i) {
		if (!a.append(i))
			return 1;
	}
	if (a.size() != 100 || a[99] != 99)
		return 2;
	int sum = 0;
	for (const int x : a)
		sum += x;
	if (sum != 4950)
		return 3;
	return a.cleanup() ? 0 : 4;
}
