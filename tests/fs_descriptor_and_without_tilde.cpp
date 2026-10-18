// expect: error delete
#include "6-resources/fs/descriptor.hpp"

auto main(void) -> int {
	auto d = ca::fs::descriptor::open("/dev/null");
	if (!d)
		return 1;
	*d &= ca::fs::append;
	return 0;
}
