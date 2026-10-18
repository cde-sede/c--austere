// expect: trap
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	volatile ca::usz i = 1;
	static_cast<void>(a.insert(i, 1));
	return 0;
}
