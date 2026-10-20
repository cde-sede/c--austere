// expect: trap
#include "1-primitives/debug/trap.hpp"
#include "1-primitives/debug/verdict.hpp"
#include "4-lifecycle/handler.hpp"
#include "4-lifecycle/trapped.hpp"
#include "4-lifecycle/with.hpp"

auto main(void) -> int {
	auto outer = ca::with(
		[](void) noexcept {
			static_cast<void>(ca::with(
				ca::handler{[](const ca::trapped&) noexcept -> ca::verdict { ca::trap("inside handles"); }},
				[](void) noexcept { ca::trap("first"); },
				[](const ca::error*) noexcept {}));
		},
		[](const ca::error*) noexcept {});
	return outer ? 1 : 2;
}
