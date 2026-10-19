// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	if (!a.append(1))
		return 1;
	if (!a.cleanup())
		return 2;
	return a.size() == 0 && a.capacity() == 0 && a.data() == nullptr ? 0 : 3;
}
