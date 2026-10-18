// expect: trap
#include "5-collections/map/map.hpp"

auto main(void) -> int {
	ca::map<int, int> m;
	if (!m.set(1, 1))
		return 1;
	return m[2];
}
