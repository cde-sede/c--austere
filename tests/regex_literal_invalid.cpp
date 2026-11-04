// expect: error regex_has_unbalanced_paren
#include "5-collections/regex/regex.hpp"

auto main(void) -> int {

	auto made = ca::regex::make("a)b");
	return made && made->cleanup() ? 0 : 1;
}
