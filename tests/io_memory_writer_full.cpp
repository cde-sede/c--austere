// expect: exit 0
// link: both
#include "3-system/error/errc.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/io/memory_writer.hpp"

auto main(void) -> int {
	char buffer[8];
	ca::io::memory_writer m{buffer};
	auto fits = ca::format_to(m, "{}{}", 1234, "56");
	if (!fits || *fits != 6U || m.view() != ca::sv::str{"123456"} || !m.flush())
		return 1;
	auto over = ca::format_to(m, "{} and more", 78);
	if (over || over.error() != ca::errc::no_space || m.view() != ca::sv::str{"123456"} || m.error() != ca::errc::no_space)
		return 2;
	auto failed = m.flush();
	if (failed || failed.error() != ca::errc::no_space || !m.flush())
		return 3;
	auto written = m.write(ca::sv::str{"789"});
	if (written || m.view() != ca::sv::str{"123456"})
		return 4;
	m.clear_error();
	if (!m.flush() || !m.write(ca::sv::str{"78"}) || m.view() != ca::sv::str{"12345678"})
		return 5;
	m.clear();
	auto hex = ca::format_to(m, "{x}", 0xabcdef);
	return hex && m.view() == ca::sv::str{"abcdef"} ? 0 : 6;
}
