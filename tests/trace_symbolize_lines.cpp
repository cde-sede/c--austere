// expect: exit 0
// mode: debug
#include "1-primitives/debug/trap.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/trace/location.hpp"
#include "3-system/trace/of.hpp"
#include "4-lifecycle/with.hpp"
#include "6-resources/trace/symbolizer.hpp"

constexpr ca::u32 TRAP_LINE = __LINE__ + 3U;

[[gnu::noinline]] static auto deepest(void) noexcept -> void {
	ca::trap("deep");
	asm volatile("" ::: "memory");
}

constexpr ca::u32 CALL_LINE = __LINE__ + 3U;

[[gnu::noinline]] static auto middle(void) noexcept -> void {
	deepest();
	asm volatile("" ::: "memory");
}

auto main(void) -> int {
	auto trapped = ca::with([](void) noexcept { middle(); }, [](const ca::error*) noexcept {});
	if (trapped)
		return 1;
	const auto addresses = ca::trace::of(trapped.error()).backtrace();
	if (addresses.size() < 2U)
		return 2;
	auto symbols = ca::trace::symbolizer::make();
	if (!symbols)
		return 3;
	ca::trace::location found[ca::trace::location::SCOPES];
	if (symbols->lookup(addresses[0], found) != 2U)
		return 4;
	const ca::trace::location& trap = found[0];
	if (trap.function() != ca::sv::str{"trap"} || trap.scopes().size() != 1U || trap.scopes()[0] != ca::sv::str{"ca"} || !trap.inlined())
		return 5;
	if (!trap.file().ends_with(ca::sv::str{"trap.hpp"}) || trap.line() == 0U || trap.offset().holds())
		return 6;
	const ca::trace::location& caller = found[1];
	if (caller.function() != ca::sv::str{"deepest"} || caller.inlined() || !caller.offset().holds())
		return 7;
	if (!caller.file().ends_with(ca::sv::str{"trace_symbolize_lines.cpp"}) || caller.line() != TRAP_LINE)
		return 8;
	if (symbols->lookup(addresses[1], found) != 1U || found[0].function() != ca::sv::str{"middle"} || found[0].line() != CALL_LINE)
		return 9;
	return symbols->cleanup() ? 0 : 10;
}
