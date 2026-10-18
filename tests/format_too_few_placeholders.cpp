// expect: error format_has_fewer_placeholders_than_arguments
#include "6-resources/print/print.hpp"

auto main(void) -> int {
	ca::print("{}\n", 1, 2);
	return 0;
}
