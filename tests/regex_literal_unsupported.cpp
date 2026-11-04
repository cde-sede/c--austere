// expect: error regex_backreferences_are_unsupported
#include "5-collections/regex/regex.hpp"

auto main(void) -> int {

	auto made = ca::regex::make("(a)\\1");
	return made && made->cleanup() ? 0 : 1;
}
