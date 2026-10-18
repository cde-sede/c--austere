// expect: error open-time options cannot change after open
#include "6-resources/fs/descriptor.hpp"

auto main(void) -> int {
	auto d = ca::fs::descriptor::open("/dev/null");
	if (!d)
		return 1;
	auto set = d->set(ca::fs::nonblock | ca::fs::create);
	return set ? 0 : 2;
}
