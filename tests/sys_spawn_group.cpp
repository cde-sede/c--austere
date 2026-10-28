// expect: exit 0
// link: both
#include "3-system/error/errc.hpp"
#include "3-system/process/getpgid.hpp"
#include "3-system/process/getpid.hpp"
#include "3-system/process/posix_spawn.hpp"
#include "3-system/process/posix_spawnp.hpp"
#include "3-system/process/spawn_options.hpp"
#include "3-system/process/try_waitpid.hpp"
#include "3-system/process/waitpid.hpp"

static_assert(!ca::sys::spawn_options{}.process_group());
static_assert(ca::sys::spawn_options{}.set_process_group(0).process_group() == 0);

auto main(void) -> int {
	char sh[] = "sh";
	char dash_c[] = "-c";
	char exit3[] = "exit 3";
	char* const exiting[]{sh, dash_c, exit3, nullptr};

	auto leader = ca::sys::posix_spawnp("sh", exiting, nullptr, nullptr, ca::sys::spawn_options{}.set_process_group(0));
	if (!leader)
		return 1;
	auto group = ca::sys::getpgid(*leader);
	if (!group || *group != *leader)
		return 2;
	auto member = ca::sys::posix_spawn("/bin/sh", exiting, nullptr, nullptr, ca::sys::spawn_options{}.set_process_group(*leader));
	if (!member)
		return 3;
	auto joined = ca::sys::getpgid(*member);
	if (!joined || *joined != *leader)
		return 4;

	auto plain = ca::sys::posix_spawnp("sh", exiting, nullptr, nullptr);
	if (!plain)
		return 5;
	auto inherited = ca::sys::getpgid(*plain);
	auto ours = ca::sys::getpgid(ca::sys::getpid());
	if (!inherited || !ours || *inherited != *ours)
		return 6;
	bool saw_leader = false;
	bool saw_member = false;
	for (int i = 0; i != 2; ++i) {
		auto ended = ca::sys::waitpid(-*leader);
		if (!ended || ended->exit_code() != 3)
			return 7;
		saw_leader = saw_leader || ended->child() == *leader;
		saw_member = saw_member || ended->child() == *member;
	}
	if (!saw_leader || !saw_member)
		return 8;

	for (;;) {
		auto ended = ca::sys::try_waitpid(-1);
		if (!ended)
			return 9;
		if (!*ended)
			continue;
		if ((*ended)->child() != *plain)
			return 9;
		break;
	}

	auto absent = ca::sys::try_waitpid(-1);
	if (absent || absent.error() != ca::errc::no_child)
		return 10;

	auto foreign = ca::sys::posix_spawnp("sh", exiting, nullptr, nullptr, ca::sys::spawn_options{}.set_process_group(1));
	if (foreign || foreign.error() != ca::errc::not_permitted)
		return 11;
	return 0;
}
