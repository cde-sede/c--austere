// expect: error no matching|no viable|could not convert|conversion from
#include "2-vocabulary/span.hpp"

auto main(void) -> int {
	const int values[2] = {1, 2};
	const ca::span<const int> constant{values};
	const ca::span<int> writable = constant;
	return writable[0];
}
