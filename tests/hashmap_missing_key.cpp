// expect: trap
#include "5-collections/map/hashmap.hpp"

auto main(void) -> int {
	ca::hashmap<int, int> m;
	if (!m.set(1, 1))
		return 1;
	return m[2];
}
