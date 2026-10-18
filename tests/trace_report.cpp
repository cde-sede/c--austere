// expect: exit 0
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/fail.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/io/memory_writer.hpp"
#include "5-collections/trace/context.hpp"
#include "5-collections/trace/report.hpp"

auto open(const ca::sv::str path) noexcept -> ca::result<int> {
	if (path == ca::sv::str{"/etc/app.conf"})
		return ca::trace::fail(ca::errc::access_denied, "open");
	return 4;
}

auto read_config(const ca::sv::str path) noexcept -> ca::result<int, ca::report<>> {
	auto file = open(path);
	if (!file)
		return ca::context(file.error(), "reading {}", path);
	return file.value();
}

auto load(const int attempt) noexcept -> ca::result<int, ca::report<>> {
	auto config = read_config("/etc/app.conf");
	if (!config)
		return ca::context(ca::move(config).error(), "loading settings, attempt {}", attempt);
	return config.value();
}

auto main(void) -> int {
	auto r = load(2);
	if (r)
		return 1;
	ca::report<>& report = r.error();
	if (report.error() != ca::errc::access_denied || report.size() != 3U || report.dropped() != 0U)
		return 2;
	if (report.frame(1).message() != ca::sv::str{"reading /etc/app.conf"})
		return 3;

	char buffer[512];
	ca::io::memory_writer m{buffer};
	if (!ca::format_to(m, "{}", report))
		return 8;
	const ca::sv::str expected{
		"Permission denied\n"
		"  at open (tests/trace_report.cpp:12): open\n"
		"  at read_config (tests/trace_report.cpp:19): reading /etc/app.conf\n"
		"  at load (tests/trace_report.cpp:26): loading settings, attempt 2"};
	if (m.view() != expected)
		return 4;

	auto plain = ca::report<>::of(ca::errc::exists);
	if (plain.size() != 0U)
		return 5;
	plain.context("no trace");
	if (plain.size() != 1U || plain.frame(0).where().line() != 55U)
		return 6;

	auto destroyed = report.destroy();
	auto plain_destroyed = plain.destroy();
	return destroyed && plain_destroyed ? 0 : 7;
}
