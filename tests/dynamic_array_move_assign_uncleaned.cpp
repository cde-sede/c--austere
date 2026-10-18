// expect: trap
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<ca::dynamic_array<int>> outer;
	ca::dynamic_array<ca::dynamic_array<int>> other;
	if (!outer.emplace())
		return 1;
	outer = ca::move(other);
	return 0;
}
