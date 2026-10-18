// expect: error is_displayable
#include "6-resources/print/print.hpp"

struct opaque {
	int value;
};

auto main(void) -> int {
	ca::print("{}\n", opaque{1});
	return 0;
}
