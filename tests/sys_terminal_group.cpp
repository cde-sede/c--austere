// expect: exit 0
// link: both
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/open.hpp"
#include "3-system/io/pipe.hpp"
#include "3-system/io/tcgetpgrp.hpp"
#include "3-system/io/tcsetpgrp.hpp"
#include "3-system/process/exit_immediately.hpp"
#include "3-system/process/fork.hpp"
#include "3-system/process/getpid.hpp"
#include "3-system/process/setsid.hpp"
#include "3-system/process/waitpid.hpp"
#include "3-system/syscall/syscalls.hpp"

static constexpr int read_write = 02;
static constexpr int no_controlling_terminal = 0400;
static constexpr unsigned long unlock_pty = 0x40045431;
static constexpr unsigned long open_peer = 0x5441;

static auto in_new_session(void) -> int {

	if (!ca::sys::setsid())
		return 1;
	auto master = ca::sys::open("/dev/ptmx", read_write | no_controlling_terminal, 0U);
	if (!master)
		return 2;
	int locked = 0;
	if (!ca::syscall::ioctl(*master, unlock_pty, &locked))
		return 3;
	auto peer = ca::syscall::ioctl(*master, open_peer, read_write);
	if (!peer)
		return 4;
	const int terminal = static_cast<int>(*peer);
	const ca::i32 self = ca::sys::getpid();
	auto group = ca::sys::tcgetpgrp(terminal);
	if (!group || *group != self)
		return 5;
	if (!ca::sys::tcsetpgrp(terminal, self))
		return 6;
	auto foreign = ca::sys::tcsetpgrp(terminal, 1);
	if (foreign || foreign.error() != ca::errc::not_permitted)
		return 7;
	return 0;
}

auto main(void) -> int {
	auto ends = ca::sys::pipe(0);
	if (!ends)
		return 1;
	auto not_terminal = ca::sys::tcgetpgrp(ends->read_end());
	if (not_terminal || not_terminal.error() != ca::errc::not_a_terminal)
		return 2;
	auto not_set = ca::sys::tcsetpgrp(ends->read_end(), ca::sys::getpid());
	if (not_set || not_set.error() != ca::errc::not_a_terminal)
		return 3;
	if (!ca::sys::close(ends->read_end()) || !ca::sys::close(ends->write_end()))
		return 4;

	auto child = ca::sys::fork();
	if (!child)
		return 5;
	if (*child == 0)
		ca::sys::exit_immediately(in_new_session());
	auto ended = ca::sys::waitpid(*child);
	if (!ended || ended->child() != *child)
		return 6;
	if (ended->signal())
		return 100 + *ended->signal();
	return ended->exit_code() == 0 ? 0 : 10 + *ended->exit_code();
}
