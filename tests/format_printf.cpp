// expect: exit 0
#include "1-primitives/integers/i8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "5-collections/format/printf_to.hpp"
#include "5-collections/io/memory_writer.hpp"

auto main(void) -> int {
	char buffer[256];
	ca::io::memory_writer m{buffer};
	const ca::i8 small = -1;
	if (!ca::printf_to(m, "%d %i %5d|%-5d|%05d %+d % d %.3d %u|", 1, 2, 3, 4, -5, 6, 7, 8, -1)
	 || !ca::printf_to(m, "%x %X %#x %o %#o %x %hhd %lu %zu %lld|", 255, 255, 255, 8, 8, small, 9, 10UL, ca::usz{11}, 12LL)
	 || !ca::printf_to(m, "%c %s %10s|%-4s|%.2s %s %s|", 'z', "str", "right", "l", "cut", true, nullptr)
	 || !ca::printf_to(m, "%.2f %e %g %G %8.3f %-8.1e| 100%%", 3.14159, 1234.5, 0.0001, 1e-10, 2.0, 5.0))
		return 2;
	return m.view() == ca::sv::str{"1 2     3|4    |-0005 +6  7 008 4294967295|"
		"ff FF 0xff 10 010 ff 9 10 11 12|"
		"z str      right|l   |cu true 0x0|"
		"3.14 1.234500e+03 0.0001 1E-10    2.000 5.0e+00 | 100%"} ? 0 : 1;
}
