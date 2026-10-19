// expect: exit 0
// link: both
#include "3-system/environment/environ.hpp"
#include "3-system/environment/setenv.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/fs/options.hpp"
#include "3-system/io/get_terminal_size.hpp"
#include "3-system/io/isatty.hpp"
#include "3-system/io/pipe.hpp"
#include "3-system/machine/gethostname.hpp"
#include "3-system/machine/processor_count.hpp"
#include "3-system/machine/uname.hpp"
#include "3-system/random/getentropy.hpp"

auto main(void) -> int {
	if (ca::sys::processor_count() < 1U)
		return 1;

	char host[256]{};
	auto length = ca::sys::gethostname(host);
	auto names = ca::sys::uname();
	if (!length || !names || ca::sv::str{host, *length} != names->node())
		return 2;
	if (names->system().empty() || names->release().empty() || names->machine().empty())
		return 3;
	char tiny[1]{};
	auto cramped = ca::sys::gethostname(tiny);
	if (cramped || (cramped.error() != ca::errc::out_of_range && cramped.error() != ca::errc::name_too_long))
		return 4;

	unsigned char random[1000]{};
	if (!ca::sys::getentropy(random))
		return 5;
	ca::usz zeros = 0U;
	for (const unsigned char byte : random)
		zeros += byte == 0U ? 1U : 0U;
	if (zeros > 40U)
		return 6;

	auto ends = ca::sys::pipe(ca::fs::cloexec_t::OPEN_VALUE);
	if (!ends || ca::sys::isatty(ends->read_end()) || ca::sys::isatty(-1))
		return 7;
	auto size = ca::sys::get_terminal_size(ends->read_end());
	if (size || !ca::sys::close(ends->read_end()) || !ca::sys::close(ends->write_end()))
		return 8;

	if (!ca::sys::setenv("CA_SYS_MACHINE", "found", true))
		return 9;
	bool found = false;
	for (char* const* entry = ca::sys::environ(); *entry != nullptr; ++entry)
		found = found || ca::sv::str{*entry} == ca::sv::str{"CA_SYS_MACHINE=found"};
	return found ? 0 : 10;
}
