// expect: error format_custom_display_takes_no_spec
#include "6-resources/print/print.hpp"

struct point {
	int x;

	auto display(ca::text_writer& out) const noexcept -> void {
		ca::format_to(out, "{}", x);
	}
};

auto main(void) -> int {
	ca::print("{>8}\n", point{1});
	return 0;
}
