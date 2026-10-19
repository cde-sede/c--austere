// expect: exit 0
// link: both
#include "3-system/process/exit_immediately.hpp"
#include "3-system/process/fork.hpp"
#include "3-system/process/getegid.hpp"
#include "3-system/process/geteuid.hpp"
#include "3-system/process/getgid.hpp"
#include "3-system/process/getpgid.hpp"
#include "3-system/process/getpid.hpp"
#include "3-system/process/getrlimit.hpp"
#include "3-system/process/getrusage.hpp"
#include "3-system/process/getuid.hpp"
#include "3-system/process/setpgid.hpp"
#include "3-system/process/setrlimit.hpp"
#include "3-system/process/setsid.hpp"
#include "3-system/process/waitpid.hpp"

static auto child_checks(void) noexcept -> int {

	auto group = ca::sys::setpgid(0, 0).and_then([](void) {
		return ca::sys::getpgid(0);
	});
	if (!group || *group != ca::sys::getpid())
		return 1;
	auto refused = ca::sys::setsid();
	if (refused)
		return 2;
	return 0;
}

auto main(void) -> int {
	if (ca::sys::geteuid() != ca::sys::getuid() || ca::sys::getegid() != ca::sys::getgid())
		return 1;
	auto own = ca::sys::getpgid(0);
	if (!own || *own <= 0)
		return 2;

	auto child = ca::sys::fork();
	if (!child)
		return 3;
	if (*child == 0)
		ca::sys::exit_immediately(child_checks());
	auto ended = ca::sys::waitpid(*child);
	if (!ended || !ended->succeeded())
		return 4;

	volatile ca::u64 spin = 0U;
	for (ca::u64 i = 0U; i < 2000000U; ++i)
		spin = spin + i;
	auto usage = ca::sys::getrusage(ca::sys::usage_target::process);
	if (!usage || usage->user_time() + usage->system_time() <= 0 || usage->max_resident() < 4096U)
		return 5;
	auto children = ca::sys::getrusage(ca::sys::usage_target::children);
	if (!children)
		return 6;

	auto files = ca::sys::getrlimit(ca::sys::resource::open_files);
	if (!files || files->current() == 0U || files->current() > files->maximum())
		return 7;
	const ca::sys::resource_limit lowered{files->current() - 1U, files->maximum()};
	if (!ca::sys::setrlimit(ca::sys::resource::open_files, lowered))
		return 8;
	auto now = ca::sys::getrlimit(ca::sys::resource::open_files);
	if (!now || now->current() != files->current() - 1U || !ca::sys::setrlimit(ca::sys::resource::open_files, *files))
		return 9;
	const ca::sys::resource_limit above{ca::sys::resource_limit::UNLIMITED, files->maximum()};
	if (files->maximum() != ca::sys::resource_limit::UNLIMITED && ca::sys::setrlimit(ca::sys::resource::open_files, above))
		return 10;
	return 0;
}
