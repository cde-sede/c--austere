// expect: trap
#include "5-collections/list/list.hpp"

auto main(void) -> int {
	ca::list<int> l;
	if (!l.append(1))
		return 1;
	return *l.end();
}
