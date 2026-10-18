// expect: trap
#include "5-collections/array/array.hpp"

static volatile ca::usz index = 3U;

auto main(void) -> int {
	ca::array<int, 3> values{1, 2, 3};
	return values[index];
}
