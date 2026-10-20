// expect: trap
#include "1-primitives/debug/trap.hpp"
#include "1-primitives/debug/verdict.hpp"
#include "4-lifecycle/handler.hpp"
#include "4-lifecycle/trapped.hpp"
#include "4-lifecycle/with.hpp"

auto main(void) -> int {
	auto passed = ca::with(
		ca::handler{[](const ca::trapped&) noexcept { return ca::verdict::pass; }},
		[](void) noexcept { ca::trap("nobody takes it"); },
		[](const ca::error*) noexcept {});
	return passed ? 1 : 2;
}
