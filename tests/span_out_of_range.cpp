// expect: trap
#include "2-vocabulary/span.hpp"

auto main(void) -> int {
	int values[2] = {1, 2};
	const ca::span<int> view{values};
	volatile ca::usz i = 2;
	return view[i];
}
