// expect: exit 0
// link: both
#include "3-system/process/exit_immediately.hpp"
#include "3-system/process/sigaction.hpp"
#include "3-system/process/sigaltstack.hpp"
#include "3-system/process/signals.hpp"

static char alternate[65536];
static volatile int keep_going = 1;

static auto on_overflow(int) -> void {
	ca::sys::exit_immediately(0);
}

[[gnu::noinline]] static auto descend(const int depth) -> int {
	volatile char frame[4096];
	frame[0] = static_cast<char>(depth);
	return keep_going != 0 ? descend(depth + 1) + frame[0] : frame[0];
}

auto main(void) -> int {
	if (!ca::sys::sigaltstack(ca::span<char>{alternate}))
		return 1;
	if (!ca::sys::sigaction(ca::sys::signals::segv, on_overflow, ca::sys::sigaction_flags::on_alternate_stack))
		return 2;
	return descend(0) == 0 ? 3 : 4;
}
