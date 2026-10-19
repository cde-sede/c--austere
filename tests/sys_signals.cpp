// expect: exit 0
// link: both
#include "3-system/process/mask_change.hpp"
#include "3-system/process/raise.hpp"
#include "3-system/process/sigaction.hpp"
#include "3-system/process/signal_disposition.hpp"
#include "3-system/process/signal_set.hpp"
#include "3-system/process/signals.hpp"
#include "3-system/process/sigprocmask.hpp"
#include "3-system/process/sigwait.hpp"

static volatile int caught = 0;

static auto on_signal(const int signal) -> void {
	caught = signal;
}

auto main(void) -> int {
	constexpr ca::sys::signal_set empty;
	static_assert(empty.empty() && !empty.contains(0) && !empty.contains(65));
	ca::sys::signal_set set;
	set.add(ca::sys::signals::user1).add(ca::sys::signals::user2).remove(ca::sys::signals::user2);
	if (!set.contains(ca::sys::signals::user1) || set.contains(ca::sys::signals::user2) || set.empty())
		return 1;

	if (!ca::sys::sigaction(ca::sys::signals::user1, on_signal) || !ca::sys::raise(ca::sys::signals::user1))
		return 2;
	if (caught != ca::sys::signals::user1)
		return 3;
	if (!ca::sys::sigaction(ca::sys::signals::user1, ca::sys::signal_disposition::ignore) || !ca::sys::raise(ca::sys::signals::user1))
		return 4;

	if (!ca::sys::sigaction(ca::sys::signals::user1, on_signal))
		return 5;
	caught = 0;
	auto previous = ca::sys::sigprocmask(ca::sys::mask_change::block, set);
	if (!previous || previous->contains(ca::sys::signals::user1))
		return 6;
	if (!ca::sys::raise(ca::sys::signals::user1) || caught != 0)
		return 7;
	auto taken = ca::sys::sigwait(set);
	if (!taken || *taken != ca::sys::signals::user1 || caught != 0)
		return 8;
	auto blocked = ca::sys::sigprocmask(ca::sys::mask_change::replace, *previous);
	if (!blocked || *blocked != set)
		return 9;

	if (!ca::sys::sigaction(ca::sys::signals::user1, ca::sys::signal_disposition::default_action))
		return 10;
	auto bad = ca::sys::sigaction(ca::sys::signals::kill, on_signal);
	return bad ? 11 : 0;
}
