// expect: trap
// mode: debug
#include "5-collections/fs/path.hpp"

auto main(void) -> int {
	auto p = ca::fs::path::from("a/b");
	return static_cast<int>(p->name().size());
}
