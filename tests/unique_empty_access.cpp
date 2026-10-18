// expect: trap
#include "4-lifecycle/unique.hpp"

auto main(void) -> int {
	ca::unique<int> owner;
	return *owner;
}
