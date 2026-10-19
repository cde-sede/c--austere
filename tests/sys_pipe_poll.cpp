// expect: exit 0
// link: both
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/read.hpp"
#include "3-system/file/write.hpp"
#include "3-system/fs/options.hpp"
#include "3-system/io/dup2.hpp"
#include "3-system/io/pipe.hpp"
#include "3-system/io/poll.hpp"
#include "3-system/io/poll_events.hpp"

auto main(void) -> int {
	auto ends = ca::sys::pipe(ca::fs::cloexec_t::OPEN_VALUE | ca::fs::nonblock_t::OPEN_VALUE);
	if (!ends)
		return 1;
	ca::sys::poll_entry watch[1] = {ca::sys::poll_entry{ends->read_end(), ca::sys::poll_events::readable}};
	auto idle = ca::sys::poll(watch, 0);
	if (!idle || *idle != 0U || watch[0].reported() != 0)
		return 2;
	char empty{};
	auto would_block = ca::sys::read(ends->read_end(), ca::span{&empty, 1U});
	if (would_block || would_block.error() != ca::errc::try_again)
		return 3;
	if (!ca::sys::dup2(ends->write_end(), 100) || !ca::sys::write(100, ca::sv::str{"x"}) || !ca::sys::close(100))
		return 4;
	auto ready = ca::sys::poll(watch, 1000);
	if (!ready || *ready != 1U || !watch[0].has(ca::sys::poll_events::readable))
		return 5;
	char got{};
	auto read = ca::sys::read(ends->read_end(), ca::span{&got, 1U});
	if (!read || *read != 1U || got != 'x')
		return 6;
	if (!ca::sys::close(ends->write_end()))
		return 7;
	auto hangup = ca::sys::poll(watch, 1000);
	if (!hangup || !watch[0].has(ca::sys::poll_events::hangup))
		return 8;
	return ca::sys::close(ends->read_end()) ? 0 : 9;
}
