// expect: error format_type_does_not_fit_the_argument
#include "6-resources/print/print.hpp"

auto main(void) -> int {
	ca::print("{x}\n", "text");
	return 0;
}
