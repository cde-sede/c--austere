// expect: trap
#include "6-resources/fs/descriptor.hpp"

auto main(void) -> int {
	auto a = ca::fs::descriptor::open("/dev/null");
	auto b = ca::fs::descriptor::open("/dev/null");
	if (!a || !b)
		return 1;
	*a = static_cast<ca::fs::descriptor&&>(*b);
	return 0;
}
