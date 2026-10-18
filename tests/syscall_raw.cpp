// expect: exit 0
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/getpid.hpp"
#include "3-system/process/signals.hpp"
#include "3-system/syscall/syscalls.hpp"

static_assert(ca::syscall::has_table);

enum class nothing : int {
	none = 0,
};

auto main(void) -> int {
	auto pid = ca::syscall::getpid();
	if (!pid || *pid != ca::sys::getpid())
		return 1;
	auto same = ca::syscall::invoke(ca::syscall::number::getpid);
	if (!same || *same != *pid)
		return 2;

	int ends[2]{-1, -1};
	if (!ca::syscall::pipe2(ends, 0))
		return 3;
	auto written = ca::syscall::write(ends[1], "ok", 2U);
	char got[2]{};
	auto read = ca::syscall::read(ends[0], got, sizeof got);
	if (!written || *written != 2 || !read || *read != 2 || got[0] != 'o' || got[1] != 'k')
		return 4;
	if (!ca::syscall::close(ends[0]) || !ca::syscall::close(ends[1]))
		return 5;

	auto closed = ca::syscall::close(-1);
	if (closed || closed.error() != ca::errc::bad_descriptor || ca::sys::last_error() != ca::errc::bad_descriptor)
		return 6;
	auto fault = ca::syscall::clock_gettime(0, nullptr);
	if (fault || fault.error() != ca::errc::bad_address || ca::sys::last_error() != ca::errc::bad_address)
		return 7;
	auto alive = ca::syscall::kill(*pid, nothing::none);
	if (!alive || *alive != 0)
		return 8;
	auto unknown = ca::syscall::invoke(100000);
	if (unknown || unknown.error() != ca::errc::not_implemented || ca::sys::last_error() != ca::errc::not_implemented)
		return 9;
	ca::sys::set_last_error(ca::error{0});
	if (ca::sys::last_error() != ca::error{0})
		return 10;
	return 0;
}
