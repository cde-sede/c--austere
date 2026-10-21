// expect: exit 0
#include "5-collections/map/hashmap.hpp"
#include "5-collections/map/is_map.hpp"
#include "5-collections/string/str.hpp"

static_assert(ca::is_map<ca::hashmap<ca::str, ca::str>>);
static_assert(ca::is_same<ca::hashmap<ca::str, int>::lookup_type, ca::sv::str>);
static_assert(ca::hashmap<ca::str, int>::TRIVIALLY_RELOCATABLE);

auto main(void) -> int {

	ca::hashmap<ca::str, ca::str> m;
	if (!m.set("name", "ca") || !m.set("language", "c++") || !m.set(ca::sv::str{"name"}, "ca-lib"))
		return 1;
	if (m.size() != 2U || m["name"] != "ca-lib" || !m.contains(ca::sv::str{"language"}) || m.contains("missing"))
		return 2;
	auto key = ca::str::make("a key long enough to live on the heap");
	if (!key || !m.set(ca::move(*key), "moved in") || m["a key long enough to live on the heap"] != "moved in")
		return 3;
	auto owned = ca::str::make("name");
	if (!owned || m[*owned] != "ca-lib" || !owned->cleanup())
		return 4;
	for (int i = 0; i < 200; ++i) {
		char text[] = {'k', static_cast<char>('a' + i % 26), static_cast<char>('a' + i / 26), '\0'};
		if (!m.set(ca::sv::str{text}, ca::sv::str{text}))
			return 5;
	}
	if (m.size() != 203U || m["kba"] != "kba")
		return 6;
	auto copy = m.clone();
	if (!copy || !(*copy == m))
		return 7;
	auto value = m.pop("language");
	if (!value || *value != "c++" || !value->cleanup() || m.contains("language"))
		return 8;
	auto item = m.popitem();
	if (!item || !item->cleanup())
		return 9;
	return m.cleanup() && copy->cleanup() ? 0 : 10;
}
