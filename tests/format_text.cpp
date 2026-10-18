// expect: exit 0
#include "3-system/error/errc.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/fs/path.hpp"
#include "5-collections/io/memory_writer.hpp"

enum class color : unsigned char { red = 3 };

auto main(void) -> int {
	auto p = ca::fs::path::from("a/b");
	if (!p)
		return 1;
	char array[8] = {'x', 'y', '\0', 'z'};
	const char* none = nullptr;
	char buffer[256];
	ca::io::memory_writer m{buffer};
	auto first = ca::format_to(m, "{} [{}] {} {} {} {}|", "lit", ca::sv::str{"view"}, array, *p, none, color::red);
	auto second = ca::format_to(m, "[{.2}] [{>5}] [{^7}] {{x}} }}|", "truncate", "r", "mid");
	auto third = ca::format_to(m, "{} {d} {} {p} {}", ca::errc::no_entry, ca::errc::no_entry, nullptr, static_cast<const char*>(nullptr), 1 < 2);
	const bool ok = first && second && third && *first + *second + *third == m.size() && m.view() == ca::sv::str{"lit [view] xy a/b (null) 3|[tr] [    r] [  mid  ] {x} }|No such file or directory 2 0x0 0x0 true"};
	return p->destroy() && ok ? 0 : 2;
}
