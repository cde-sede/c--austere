// expect: exit 0
#include "5-collections/format/format_to.hpp"
#include "5-collections/format/printf_to.hpp"
#include "5-collections/io/memory_writer.hpp"

struct point {
	int x;
	int y;

	auto display(ca::text_writer& out) const noexcept -> void {
		ca::format_to(out, "({}, {})", x, y);
	}
};

struct segment {
	point from;
	point to;

	auto display(ca::text_writer& out) const noexcept -> void {
		ca::format_to(out, "{} -> {}", from, to);
		out.put('!');
	}
};

auto main(void) -> int {
	char buffer[64];
	ca::io::memory_writer m{buffer};
	auto first = ca::format_to(m, "{} ", segment{{1, 2}, {3, -4}});
	auto second = ca::printf_to(m, "%s", point{5, 6});
	if (!first || *first != 19U || !second || *second != 6U)
		return 2;
	return m.view() == ca::sv::str{"(1, 2) -> (3, -4)! (5, 6)"} ? 0 : 1;
}
