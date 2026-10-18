// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	if (a.pop())
		return 1;
	if (!a.append(1) || !a.append(2))
		return 2;
	auto last = a.pop();
	if (!last || *last != 2 || a.size() != 1)
		return 3;
	return a.destroy() ? 0 : 4;
}
