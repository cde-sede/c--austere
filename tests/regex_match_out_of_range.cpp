// expect: trap
#include "5-collections/regex/regex.hpp"

auto main(void) -> int {

	auto made = ca::regex::make("(a)b");
	if (!made)
		return 1;
	auto found = made->search("ab");
	if (!found || !*found)
		return 2;
	return static_cast<int>((**found)[2U].size());
}
