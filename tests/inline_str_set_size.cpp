// expect: trap
#include "5-collections/string/inline_str.hpp"

auto main(void) -> int {
	ca::inline_str<4> s;
	volatile ca::usz size = 5;
	s.set_size(size);
	return 0;
}
