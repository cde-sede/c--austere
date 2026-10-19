// expect: exit 0
#include "6-resources/fs/descriptor.hpp"

auto main(void) -> int {
	auto d = ca::fs::descriptor::open("/dev/null", ca::fs::writable | ca::fs::append | ca::fs::cloexec);
	if (!d)
		return 1;
	*d |= ca::fs::nonblock;
	auto all = d->test(ca::fs::nonblock | ca::fs::append | ca::fs::cloexec);
	if (!all || !*all)
		return 2;
	*d &= ~(ca::fs::nonblock | ca::fs::cloexec);
	auto kept = d->test(ca::fs::append);
	auto gone = d->test(ca::fs::nonblock);
	auto closed = d->test(ca::fs::cloexec);
	if (!kept || !*kept || !gone || *gone || !closed || *closed)
		return 3;
	return d->cleanup() ? 0 : 4;
}
