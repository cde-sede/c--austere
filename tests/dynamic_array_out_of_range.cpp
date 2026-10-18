// expect: trap
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	if (!a.append(1))
		return 1;
	volatile ca::usz i = 1;
	return a[i];
}
