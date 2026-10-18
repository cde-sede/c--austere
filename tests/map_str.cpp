// expect: exit 0
#include "5-collections/map/is_map.hpp"
#include "5-collections/map/map.hpp"
#include "5-collections/string/str.hpp"

static_assert(ca::is_map<ca::map<ca::str, ca::str>>);

auto main(void) -> int {

	ca::map<ca::str, int> m;
	const char* const words[] = {"pear", "apple", "fig", "kiwi", "banana", "cherry", "date", "grape", "lemon", "mango", "nectarine", "orange", "plum", "quince"};
	int i = 0;
	for (const char* const word : words)
		if (!m.set(word, i++))
			return 1;
	if (m.size() != 14U || m["fig"] != 2 || !m.contains(ca::sv::str{"quince"}) || m.contains("zucchini"))
		return 2;
	ca::sv::str previous{""};
	for (const auto [key, value] : m) {
		if (key.view().compare(previous) <= 0)
			return 3;
		previous = key.view();
	}
	auto first = m.first();
	if (!first || first->key != "apple" || m.ceiling("c").value_or(*first).key != "cherry" || m.floor("c").value_or(*first).key != "banana")
		return 4;
	auto copy = m.clone();
	if (!copy || !(*copy == m))
		return 5;
	if (m.pop("kiwi").value_or(-1) != 3 || m.contains("kiwi"))
		return 6;
	auto top = m.popitem();
	if (!top || top->key != "quince" || !top->destroy())
		return 7;
	return m.destroy() && copy->destroy() ? 0 : 8;
}
