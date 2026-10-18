// expect: exit 0
#include "2-vocabulary/sv/str.hpp"

static_assert(ca::sv::str{"abc"}.size() == 3U);
static_assert(ca::sv::str{"a/b/c"}.find_last('/') == 3U);
static_assert(ca::sv::str{"hello"}.prefix(2U) == ca::sv::str{"he"});
static_assert(ca::sv::str{"hello"}.suffix(9U) == ca::sv::str{"hello"});
static_assert(ca::sv::str{"hello"}.without_prefix(9U).empty());
static_assert(ca::sv::str{"file.tar.gz"}.ends_with(ca::sv::str{".gz"}));

auto main(void) -> int {
	const ca::sv::str s{"abc"};
	if (s.find('z') || !s.starts_with(ca::sv::str{"ab"}))
		return 1;
	if (s.get(3U))
		return 2;
	return s.without_suffix(1U) == ca::sv::str{"ab"} ? 0 : 3;
}
