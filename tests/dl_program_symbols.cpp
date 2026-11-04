// expect: exit 0
// ldflags: -Wl,--export-dynamic
// shared: program dl/program.cpp
// shared: program_host dl/program.cpp
// shared: program_tls dl/program_tls.cpp
// A library's undefined symbols resolve against what the program exports, without host
// symbols, in a libc program (whose dynamic section ld.so rewrote) and a static PIE; host
// symbols still come first; the program's thread-local variables are refused. A static non-PIE
// program has no dynamic section even with --export-dynamic (GNU ld), so it exports nothing.
#include "2-vocabulary/sv/str.hpp"
#include "3-system/elf/hashed_name.hpp"
#include "3-system/elf/image.hpp"
#include "3-system/elf/program_header.hpp"
#include "3-system/error/errc.hpp"
#include "5-collections/format/format.hpp"
#include "5-collections/trace/report.hpp"
#include "6-resources/dl/host_symbol.hpp"
#include "6-resources/dl/library.hpp"

extern "C" {

	[[gnu::used, gnu::visibility("default")]] int program_counter = 100;

	[[gnu::used, gnu::visibility("default"), gnu::noinline]] auto program_add(const int value) -> int {

		asm("");
		return value + 1;
	}

	[[gnu::used, gnu::visibility("default"), gnu::noinline]] auto program_shadowed(void) -> int {

		asm("");
		return 1;
	}

	[[gnu::used, gnu::visibility("default")]] constinit thread_local int program_tls = 7;

}

static auto host_shadowed(void) -> int {
	return 2;
}

static auto exported(void) noexcept -> int {
	const auto program = ca::elf::image::program();
	if (!program)
		return 1;
	const auto add = program->find(ca::elf::hashed_name::of(ca::sv::str{"program_add"}));
	if (!add || program->address(add->_value) != reinterpret_cast<ca::u64>(&program_add))
		return 2;
	const auto counter = program->find(ca::elf::hashed_name::of(ca::sv::str{"program_counter"}));
	if (!counter || program->address(counter->_value) != reinterpret_cast<ca::u64>(&program_counter))
		return 3;
	return 0;
}

static auto bound(void) noexcept -> int {
	auto plugin = ca::dl::library::make(CA_TEST_LIBRARIES "/libprogram.so");
	if (!plugin)
		return 10;
	const auto run = plugin->find_as<int(int)>("plugin_run");
	const auto bump = plugin->find_as<void(void)>("plugin_bump");
	const auto shadowed = plugin->find_as<int(void)>("plugin_shadowed");
	if (!run || !bump || !shadowed)
		return 11;
	if ((*run)(5) != 106)
		return 12;
	(*bump)();
	if (program_counter != 101 || (*run)(5) != 107)
		return 13;
	if ((*shadowed)() != 1)
		return 14;
	return plugin->cleanup() ? 0 : 15;
}

static auto host_first(void) noexcept -> int {
	const ca::dl::host_symbol host[] = {{"program_shadowed", reinterpret_cast<void*>(&host_shadowed)}};
	auto plugin = ca::dl::library::make(CA_TEST_LIBRARIES "/libprogram_host.so", host);
	if (!plugin)
		return 20;
	const auto shadowed = plugin->find_as<int(void)>("plugin_shadowed");
	const auto run = plugin->find_as<int(int)>("plugin_run");
	if (!shadowed || (*shadowed)() != 2 || !run || (*run)(1) != 2 + program_counter)
		return 21;
	return plugin->cleanup() ? 0 : 22;
}

static auto thread_local_refused(void) noexcept -> int {
	auto plugin = ca::dl::library::make(CA_TEST_LIBRARIES "/libprogram_tls.so");
	if (plugin)
		return 30;
	ca::report<>& report = plugin.error();
	auto printed = ca::format("{}", report);
	const bool explained = printed && printed->view().contains("program_tls");
	if (printed)
		static_cast<void>(printed->cleanup());
	if (report.error() != ca::errc::not_supported || !explained)
		return 31;
	return report.cleanup() && program_tls == 7 ? 0 : 32;
}

static auto unexported(void) noexcept -> int {
	auto plugin = ca::dl::library::make(CA_TEST_LIBRARIES "/libprogram.so");
	if (plugin)
		return 40;
	const bool undefined = plugin.error().error() == ca::errc::no_entry;
	return plugin.error().cleanup() && undefined ? 0 : 41;
}

auto main(void) -> int {
	const auto program = ca::elf::image::program();
	if (!program)
		return 50;
	if (!program->segment(ca::elf::segment_types::dynamic))
		return program->symbols().empty() ? unexported() : 51;
	for (auto* const check : {&exported, &bound, &host_first, &thread_local_refused}) {
		const int failed = check();
		if (failed != 0)
			return failed;
	}
	return 0;
}
