// expect: trap
#include "5-collections/array/deque.hpp"

static volatile ca::usz index = 1U;

auto main(void) -> int {
	ca::deque<int> d;
	if (!d.append(1))
		return 1;
	return d[index];
}
