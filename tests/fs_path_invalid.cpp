// expect: exit 0
#include "3-system/error/errc.hpp"
#include "5-collections/fs/path.hpp"

auto main(void) -> int {
	auto nul = ca::fs::path::from(ca::sv::str{"a\0b", 3U});
	if (nul || nul.error() != ca::errc::invalid_argument)
		return 1;
	auto p = ca::fs::path::from("a/b.txt");
	if (!p)
		return 2;
	if (p->with_name("x/y") || p->with_name("") || p->with_name(".") || p->with_suffix("txt") || p->with_suffix(".") || p->with_stem("a/b"))
		return 3;
	const ca::fs::path dot;
	if (dot.with_name("x") || dot.relative_to(*p))
		return 4;
	return 0;
}
