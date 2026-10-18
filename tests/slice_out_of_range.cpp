// expect: trap
#include "5-collections/array/slice.hpp"

auto main(void) -> int {
	auto s = ca::slice<int>::make(2U);
	if (!s)
		return 1;
	volatile ca::usz i = 2;
	return (*s)[i];
}
