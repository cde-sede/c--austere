// expect: exit 0
// link: both
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/context.hpp"
#include "3-system/trace/fail.hpp"
#include "3-system/trace/of.hpp"
#include "5-collections/io/memory_writer.hpp"
#include "6-resources/print/print_trace.hpp"

auto open_config(const bool present) noexcept -> ca::result<int> {
	if (!present)
		return ca::trace::fail(ca::errc::no_entry);
	return 3;
}

auto load(const bool present) noexcept -> ca::result<int> {
	auto config = open_config(present);
	if (!config)
		return ca::trace::context(config.error(), "opening config");
	return config.value() + 1;
}

auto start(void) noexcept -> ca::result<int> {
	auto loaded = load(false);
	if (!loaded)
		return ca::trace::context(loaded.error(), "loading settings");
	return loaded.value();
}

auto main(void) -> int {
	auto r = start();
	if (r)
		return 1;
	const ca::error e = r.error();
	if (e != ca::errc::no_entry || e.trace() == 0U)
		return 2;

	const ca::trace::record record = ca::trace::of(e);
	if (record.frames().size() != 3U || record.dropped() != 0U)
		return 3;
	if (record.frames()[1].message() != ca::sv::str{"opening config"} || record.frames()[0].where().line() != 13U)
		return 4;

	char buffer[512];
	ca::io::memory_writer m{buffer};
	if (!ca::print_trace(m, e))
		return 9;
	const ca::sv::str expected{
		"No such file or directory\n"
		"  at open_config (tests/trace_context.cpp:13)\n"
		"  at load (tests/trace_context.cpp:20): opening config\n"
		"  at start (tests/trace_context.cpp:27): loading settings\n"};
	if (m.view() != expected)
		return 5;

	if (!ca::trace::of(ca::errc::no_entry).frames().empty())
		return 6;

	const ca::error untraced = ca::trace::context(ca::errc::exists, "untraced").error();
	if (untraced.trace() == e.trace() || ca::trace::of(untraced).frames().size() != 1U)
		return 7;
	if (!ca::trace::of(e).frames().empty())
		return 8;
	return 0;
}
