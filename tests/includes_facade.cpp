// expect: exit 0
#include "ca/all.hpp"
#include "ca/dynamic_array.hpp"
#include "ca/memory.hpp"
#include "ca/print.hpp"

auto main(void) -> int {
	ca::dynamic_array<int, ca::page_allocator> values;
	if (!values.append(7))
		return 1;
	auto first = values.get(0);
	if (!first)
		return 2;
	char buffer[16];
	ca::io::memory_writer m{buffer};
	auto formatted = ca::format_to(m, "{}", *first);
	auto printed = ca::print("{}\n", m.view());
	if (!formatted || !printed || *printed != 2U)
		return 4;
	return values.destroy() && ca::flush() && m.view() == ca::sv::str{"7"} ? 0 : 3;
}
