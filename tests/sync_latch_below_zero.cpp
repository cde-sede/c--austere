// expect: trap
#include "3-system/sync/latch.hpp"

auto main(void) -> int {
	ca::latch countdown{1U};
	countdown.count_down(2U);
	return 0;
}
