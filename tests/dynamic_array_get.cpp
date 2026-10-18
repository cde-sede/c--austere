// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	if (!a.append(5))
		return 1;
	auto g = a.get(0);
	if (!g || *g != 5)
		return 2;
	*g = 50;
	if (a[0] != 50)
		return 3;
	if (a.get(1))
		return 4;
	if (a.get(1).value_or(-1) != -1)
		return 5;
	const auto& constant = a;
	auto c = constant.get(0);
	if (!c || *c != 50)
		return 6;
	return a.destroy() ? 0 : 7;
}
