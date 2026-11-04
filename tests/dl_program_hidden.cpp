// expect: exit 0
// shared: program dl/program.cpp
// A program that does not export its symbols offers none: the library's references to them stay
// undefined.
#include "2-vocabulary/sv/str.hpp"
#include "3-system/elf/hashed_name.hpp"
#include "3-system/elf/image.hpp"
#include "3-system/error/errc.hpp"
#include "5-collections/format/format.hpp"
#include "5-collections/trace/report.hpp"
#include "6-resources/dl/library.hpp"

extern "C" {

	[[gnu::used]] int program_counter = 100;

	[[gnu::used, gnu::noinline]] auto program_add(const int value) -> int {

		asm("");
		return value + 1;
	}

	[[gnu::used, gnu::noinline]] auto program_shadowed(void) -> int {

		asm("");
		return 1;
	}

}

auto main(void) -> int {
	const auto program = ca::elf::image::program();
	if (!program || program->find(ca::elf::hashed_name::of(ca::sv::str{"program_add"})))
		return 1;
	auto plugin = ca::dl::library::make(CA_TEST_LIBRARIES "/libprogram.so");
	if (plugin)
		return 2;
	ca::report<>& report = plugin.error();
	auto printed = ca::format("{}", report);
	const bool explained = printed && printed->view().contains("undefined symbol program_") && printed->view().contains("libprogram.so");
	if (printed)
		static_cast<void>(printed->cleanup());
	if (report.error() != ca::errc::no_entry || !explained)
		return 3;
	return report.cleanup() && program_add(program_counter) == 101 ? 0 : 4;
}
