// expect: trap
#include "1-primitives/debug/trap.hpp"
#include "1-primitives/debug/verdict.hpp"
#include "4-lifecycle/handler.hpp"
#include "4-lifecycle/trapped.hpp"
#include "4-lifecycle/with.hpp"

auto main(void) -> int {
	auto resumed = ca::with(
		ca::handler{[](const ca::trapped&) noexcept { return ca::verdict::resume; }},
		[](void) noexcept { ca::trap("cannot resume"); },
		[](const ca::error*) noexcept {});
	return resumed ? 1 : 2;
}
