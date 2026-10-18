// expect: error printf_star_width_or_precision_is_unsupported
#include "6-resources/print/printf.hpp"

auto main(void) -> int {
	ca::printf("%*d\n", 4, 2);
	return 0;
}
