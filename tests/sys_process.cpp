// expect: exit 0
#include "3-system/process/execvp.hpp"
#include "3-system/process/exit_immediately.hpp"
#include "3-system/process/fork.hpp"
#include "3-system/process/getpid.hpp"
#include "3-system/process/getppid.hpp"
#include "3-system/process/kill.hpp"
#include "3-system/process/signals.hpp"
#include "3-system/process/try_waitpid.hpp"
#include "3-system/process/waitpid.hpp"
#include "3-system/time/sleep_for.hpp"

auto main(void) -> int {
	const ca::i32 self = ca::sys::getpid();
	auto child = ca::sys::fork();
	if (!child)
		return 1;
	if (*child == 0)
		ca::sys::exit_immediately(ca::sys::getppid() == self ? 7 : 1);
	auto ended = ca::sys::waitpid(*child);
	if (!ended || ended->exit_code() != 7 || ended->signal())
		return 2;
	auto sleeper = ca::sys::fork();
	if (!sleeper)
		return 3;
	if (*sleeper == 0) {
		static_cast<void>(ca::sys::sleep_for(10000000000));
		ca::sys::exit_immediately(0);
	}
	auto running = ca::sys::try_waitpid(*sleeper);
	if (!running || running->has_value() || !ca::sys::kill(*sleeper, ca::sys::signals::terminate))
		return 4;
	auto killed = ca::sys::waitpid(*sleeper);
	if (!killed || killed->exited() || killed->signal() != ca::sys::signals::terminate)
		return 5;
	auto shell = ca::sys::fork();
	if (!shell)
		return 6;
	if (*shell == 0) {
		char name[] = "true";
		char* argv[] = {name, nullptr};
		static_cast<void>(ca::sys::execvp(name, argv));
		ca::sys::exit_immediately(127);
	}
	auto ran = ca::sys::waitpid(*shell);
	if (!ran || !ran->succeeded())
		return 7;
	auto none = ca::sys::waitpid(*shell);
	return none ? 8 : 0;
}
