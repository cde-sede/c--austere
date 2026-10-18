// expect: error format_type_does_not_fit_the_argument
#include "6-resources/print/printf.hpp"

auto main(void) -> int {
	ca::printf("%d\n", "not a number");
	return 0;
}
