// expect: trap
// mode: debug
#include "4-lifecycle/memory/heap.hpp"

auto main(void) -> int {
	auto block = ca::heap{}.allocate(32U, 8U);
	if (!block)
		return 1;
	ca::heap{}.deallocate(block->with_size(200U), 8U);
	return 0;
}
