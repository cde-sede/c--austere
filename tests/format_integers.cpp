// expect: exit 0
#include "5-collections/format/format_to.hpp"
#include "5-collections/io/memory_writer.hpp"

auto main(void) -> int {
	char buffer[256];
	ca::io::memory_writer m{buffer};
	auto first = ca::format_to(m, "{} {} {} {} {}|", 0, -7, 18446744073709551615ULL, -9223372036854775807LL - 1, static_cast<unsigned char>(200));
	if (!first || *first != 51U)
		return 2;
	if (!ca::format_to(m, "{x} {X} {#x} {#b} {o} {#o} {08x} {+} {+d} {.3}|", 255, 255, 255, 5, 8, 8, 0xbeef, 3, -3, 7)
	 || !ca::format_to(m, "[{5}] [{<5}] [{^5}] [{*>5}] [{-^6}] [{05}] [{<05}]|", 42, 42, 42, 42, 42, -42, 42)
	 || !ca::format_to(m, "{} {d} {c} {d} {x}", true, false, 65, 'A', -1))
		return 3;
	return m.view() == ca::sv::str{"0 -7 18446744073709551615 -9223372036854775808 200|"
		"ff FF 0xff 0b101 10 010 0000beef +3 -3 007|"
		"[   42] [42   ] [ 42  ] [***42] [--42--] [-0042] [42   ]|"
		"true 0 A 65 -1"} ? 0 : 1;
}
