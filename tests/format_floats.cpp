// expect: exit 0
#include "5-collections/format/format_to.hpp"
#include "5-collections/io/memory_writer.hpp"

auto main(void) -> int {
	char buffer[256];
	ca::io::memory_writer m{buffer};
	if (!ca::format_to(m, "{} {} {} {} {} {}|", 0.1, 1.0, 0.1F, 1e300, -0.0, 123456789.0)
	 || !ca::format_to(m, "{.3} {f} {.2f} {e} {+.1f} {08.2f} {<8.1f}|", 3.14159, 2.5, 2.555, 1234.5, 1.0, -3.14159, 1.25)
	 || !ca::format_to(m, "{} {} {08}", 1.0 / 0.0, -1.0 / 0.0, 1.0 / 0.0))
		return 2;
	return m.view() == ca::sv::str{"0.1 1 0.1 1e+300 -0 123456789|"
		"3.14 2.500000 2.56 1.234500e+03 +1.0 -0003.14 1.2     |"
		"inf -inf      inf"} ? 0 : 1;
}
