// expect: trap
#include "1-primitives/utility/move.hpp"
#include "3-system/process/spawn_actions.hpp"

auto main(void) -> int {
	auto live = ca::sys::spawn_actions::make();
	auto other = ca::sys::spawn_actions::make();
	if (!live || !other)
		return 1;
	*live = ca::move(*other);
	return 0;
}
