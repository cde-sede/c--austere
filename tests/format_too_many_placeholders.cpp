// expect: error format_has_more_placeholders_than_arguments
#include "6-resources/print/print.hpp"

auto main(void) -> int {
	ca::print("{} {}\n", 1);
	return 0;
}
