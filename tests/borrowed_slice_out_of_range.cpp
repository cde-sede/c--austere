// expect: trap
#include "2-vocabulary/borrowed_slice.hpp"

auto main(void) -> int {
	int values[2] = {1, 2};
	const ca::borrowed_slice<int> view{values};
	volatile ca::usz i = 2;
	return view[i];
}
