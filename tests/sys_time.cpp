// expect: exit 0
// link: both
#include "3-system/time/now.hpp"
#include "3-system/time/sleep_for.hpp"

auto main(void) -> int {
	const ca::i64 start = ca::sys::now(ca::sys::clock::monotonic);
	if (!ca::sys::sleep_for(2000000))
		return 1;
	const ca::i64 elapsed = ca::sys::now(ca::sys::clock::monotonic) - start;
	if (elapsed < 2000000)
		return 2;
	if (ca::sys::now(ca::sys::clock::realtime) < 1700000000LL * 1000000000LL)
		return 3;
	return ca::sys::now(ca::sys::clock::process_cpu) > 0 ? 0 : 4;
}
