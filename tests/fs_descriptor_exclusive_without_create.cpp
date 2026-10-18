// expect: error exclusive needs create
#include "6-resources/fs/descriptor.hpp"

auto main(void) -> int {
	auto d = ca::fs::descriptor::open("x", ca::fs::writable | ca::fs::exclusive);
	return d ? 0 : 1;
}
