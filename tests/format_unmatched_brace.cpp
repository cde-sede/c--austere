// expect: error format_has_unmatched_closing_brace
#include "6-resources/print/print.hpp"

auto main(void) -> int {
	ca::print("oops }\n");
	return 0;
}
