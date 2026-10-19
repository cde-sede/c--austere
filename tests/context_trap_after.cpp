// expect: trap
#include "1-primitives/debug/trap.hpp"
#include "4-lifecycle/with.hpp"

auto main(void) -> int {
	auto done = ca::with([](void) noexcept {}, [](const ca::error*) noexcept {});
	if (!done)
		return 1;
	ca::trap();
}
