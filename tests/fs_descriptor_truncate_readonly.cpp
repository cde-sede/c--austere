// expect: error truncate needs writable
#include "6-resources/fs/descriptor.hpp"

auto main(void) -> int {
	auto d = ca::fs::descriptor::open("x", ca::fs::readable | ca::fs::truncate);
	return d ? 0 : 1;
}
