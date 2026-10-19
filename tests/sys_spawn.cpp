// expect: exit 0
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/read.hpp"
#include "3-system/fs/options.hpp"
#include "3-system/io/pipe.hpp"
#include "3-system/process/posix_spawn.hpp"
#include "3-system/process/posix_spawnp.hpp"
#include "3-system/process/spawn_actions.hpp"
#include "3-system/process/waitpid.hpp"

auto main(void) -> int {
	char sh[] = "sh";
	char dash_c[] = "-c";
	char exit7[] = "exit 7";
	char* const exiting[]{sh, dash_c, exit7, nullptr};
	auto child = ca::sys::posix_spawnp("sh", exiting, nullptr, nullptr);
	if (!child)
		return 1;
	auto ended = ca::sys::waitpid(*child);
	if (!ended || ended->exit_code() != 7)
		return 2;

	auto ends = ca::sys::pipe(ca::fs::cloexec_t::OPEN_VALUE);
	if (!ends)
		return 3;
	ca::sys::spawn_actions actions;
	auto early = actions.add_close(0);
	if (early || early.error() != ca::errc::invalid_argument || actions.is_live())
		return 4;
	if (!actions.init() || !actions.add_dup2(ends->write_end(), 1) || !actions.add_open(0, "/dev/null", ca::fs::access::READ_ONLY, 0U))
		return 5;
	char echo[] = "printf \"$GREETING\"";
	char* const printing[]{sh, dash_c, echo, nullptr};
	char greeting[] = "GREETING=hi there";
	char* const environment[]{greeting, nullptr};
	auto printer = ca::sys::posix_spawn("/bin/sh", printing, environment, &actions);
	if (!printer || !actions.cleanup() || actions.is_live() || !ca::sys::close(ends->write_end()))
		return 6;
	char got[16]{};
	ca::usz total = 0U;
	for (;;) {
		auto read = ca::sys::read(ends->read_end(), ca::span<char>{got}.without_prefix(total));
		if (!read)
			return 7;
		if (*read == 0U)
			break;
		total += *read;
	}
	auto printed = ca::sys::waitpid(*printer);
	if (!printed || !printed->succeeded() || total != 8U || got[0] != 'h' || got[7] != 'e')
		return 8;

	auto stale = ca::sys::posix_spawn("/bin/sh", printing, nullptr, &actions);
	if (stale || stale.error() != ca::errc::invalid_argument)
		return 9;
	auto missing = ca::sys::posix_spawn("/nonexistent/program", exiting, nullptr, nullptr);
	if (missing) {
		auto status = ca::sys::waitpid(*missing);
		if (!status || status->exit_code() != 127)
			return 10;
	} else if (missing.error() != ca::errc::no_entry) {
		return 11;
	}
	return ca::sys::close(ends->read_end()) ? 0 : 12;
}
