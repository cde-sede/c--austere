// expect: trap
#include "2-vocabulary/sv/str.hpp"

auto main(void) -> int {
	const ca::sv::str s{"abc"};
	volatile ca::usz i = 3;
	return s[i];
}
