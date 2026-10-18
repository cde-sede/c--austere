// expect: trap
#include "4-lifecycle/memory/arena.hpp"

auto main(void) -> int {
	ca::arena arena;
	auto allocator = arena.allocator();
	return allocator.allocate(1U, 1U) ? 0 : 1;
}
