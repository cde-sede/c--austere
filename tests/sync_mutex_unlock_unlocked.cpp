// expect: trap
#include "3-system/sync/mutex.hpp"

auto main(void) -> int {
	ca::mutex m;
	m.lock();
	m.unlock();
	m.unlock();
	return 0;
}
