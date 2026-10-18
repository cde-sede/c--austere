// expect: trap
#include "5-collections/string/str.hpp"

auto main(void) -> int {
	ca::str s;
	if (!s.append("abc"))
		return 1;
	volatile ca::usz i = 3;
	return s[i];
}
