// expect: exit 0
#include "1-primitives/debug/trap.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/with.hpp"
#include "5-collections/io/memory_writer.hpp"
#include "6-resources/print/print_trace.hpp"

auto main(void) -> int {
	auto trapped = ca::with([](void) noexcept { ca::trap("boom"); }, [](const ca::error*) noexcept {});
	if (trapped)
		return 1;
	char buffer[2048];
	ca::io::memory_writer m{buffer};
	if (!ca::print_trace(m, trapped.error()))
		return 2;
	return m.view().starts_with(ca::sv::str{"State not recoverable\n  trapped: boom\n  backtrace:\n    #0 exe+0x"}) ? 0 : 3;
}
