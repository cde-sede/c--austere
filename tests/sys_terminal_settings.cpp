// expect: exit 0
// link: both
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/open.hpp"
#include "3-system/io/enter_raw_mode.hpp"
#include "3-system/io/pipe.hpp"
#include "3-system/io/tcgetattr.hpp"
#include "3-system/io/tcsetattr.hpp"
#include "3-system/syscall/syscalls.hpp"

static constexpr int read_write = 02;
static constexpr int no_controlling_terminal = 0400;
static constexpr unsigned long unlock_pty = 0x40045431;
static constexpr unsigned long open_peer = 0x5441;

static_assert(ca::sys::terminal_settings{}.raw().character(ca::sys::terminal_character::minimum) == 1U);

auto main(void) -> int {
	auto ends = ca::sys::pipe(0);
	if (!ends)
		return 1;
	auto not_terminal = ca::sys::tcgetattr(ends->read_end());
	if (not_terminal || not_terminal.error() != ca::errc::not_a_terminal)
		return 2;
	if (!ca::sys::close(ends->read_end()) || !ca::sys::close(ends->write_end()))
		return 3;

	auto master = ca::sys::open("/dev/ptmx", read_write | no_controlling_terminal, 0U);
	if (!master)
		return 4;
	int locked = 0;
	if (!ca::syscall::ioctl(*master, unlock_pty, &locked))
		return 5;
	auto peer = ca::syscall::ioctl(*master, open_peer, read_write | no_controlling_terminal);
	if (!peer)
		return 5;
	const int terminal = static_cast<int>(*peer);

	auto initial = ca::sys::tcgetattr(terminal);
	if (!initial || (initial->local() & ca::sys::terminal_local::canonical) == 0U)
		return 6;
	auto previous = ca::sys::enter_raw_mode(terminal);
	if (!previous || *previous != *initial)
		return 7;
	auto raw = ca::sys::tcgetattr(terminal);
	if (!raw || *raw != initial->raw())
		return 8;
	if ((raw->local() & (ca::sys::terminal_local::canonical | ca::sys::terminal_local::echo)) != 0U
		|| (raw->output() & ca::sys::terminal_output::process) != 0U
		|| (raw->control() & ca::sys::terminal_control::size) != ca::sys::terminal_control::size_8
		|| raw->character(ca::sys::terminal_character::minimum) != 1U)
		return 9;

	auto edited = *initial;
	edited.set_local(edited.local() & ~ca::sys::terminal_local::echo).set_character(ca::sys::terminal_character::interrupt, 7U);
	if (!ca::sys::tcsetattr(terminal, edited, ca::sys::terminal_change::drain))
		return 10;
	auto reread = ca::sys::tcgetattr(terminal);
	if (!reread || *reread != edited)
		return 11;

	if (!ca::sys::tcsetattr(terminal, *previous))
		return 12;
	auto restored = ca::sys::tcgetattr(terminal);
	if (!restored || *restored != *initial)
		return 13;
	return ca::sys::close(terminal) && ca::sys::close(*master) ? 0 : 14;
}
