// expect: exit 0
#include "2-vocabulary/error.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/context.hpp"
#include "3-system/trace/fail.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/io/memory_writer.hpp"

auto main(void) -> int {
	char buffer[256];
	ca::io::memory_writer m{buffer};

	const ca::error failed = ca::trace::fail(ca::errc::invalid_argument, "should_fail").error();
	const ca::error extended = ca::trace::context(failed, "outer").error();
	auto traced = ca::format_to(m, "{}|", extended);

	const ca::error unnamed = ca::trace::fail(ca::errc::exists).error();
	auto rest = ca::format_to(m, "{}|{}|{}|{}|{d}|{>8}", unnamed, failed, ca::errc::no_entry, ca::error{33}, ca::errc::no_entry, ca::errc::exists);
	if (!traced || !rest)
		return 1;
	return m.view() == ca::sv::str{"should_fail|exists|invalid_argument|no_entry|Numerical argument out of domain|2|  exists"} ? 0 : 2;
}
