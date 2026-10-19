// expect: exit 0
// link: both
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/context.hpp"
#include "3-system/trace/fail.hpp"
#include "3-system/trace/of.hpp"
#include "5-collections/io/memory_writer.hpp"
#include "5-collections/trace/report.hpp"
#include "6-resources/print/print_trace.hpp"

auto descend(const int depth) noexcept -> ca::result<void> {
	if (depth == 0)
		return ca::trace::fail(ca::errc::out_of_range, "bottom");
	auto deeper = descend(depth - 1);
	if (!deeper)
		return ca::trace::context(deeper.error(), "step");
	return deeper;
}

auto main(void) -> int {
	auto r = descend(39);
	if (r)
		return 1;
	const ca::trace::record record = ca::trace::of(r.error());
	if (record.frames().size() != 32U || record.dropped() != 8U)
		return 2;
	if (record.frames()[0].message() != ca::sv::str{"bottom"})
		return 3;

	char buffer[4096];
	ca::io::memory_writer m{buffer};
	auto printed = ca::print_trace(m, r.error());
	if (!printed || *printed != m.size() || !m.view().ends_with(ca::sv::str{"): step\n  ... 8 more frames\n"}))
		return 4;

	auto report = ca::report<>::of(r.error());
	if (report.size() != 32U || report.dropped() != 8U)
		return 5;
	auto destroyed = report.cleanup();
	return destroyed ? 0 : 6;
}
