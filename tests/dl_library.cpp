// expect: exit 0
// shared: base dl/base.cpp -Wl,-soname,libbase.so -Wl,--hash-style=sysv
// shared: user dl/user.cpp -Wl,-soname,libuser.so -L{dir} -lbase -Wl,-rpath,$ORIGIN -Wl,--hash-style=gnu -Wl,-z,pack-relative-relocs -Wl,-z,relro -Wl,-z,now
// shared: broken dl/broken.cpp
// shared: ghost dl/broken.cpp -Wl,-soname,libca_ghost.so
// shared: orphan dl/orphan.cpp -L{dir} -lghost -Wl,-rpath,$ORIGIN
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/elf/program_header.hpp"
#include "3-system/error/errc.hpp"
#include "5-collections/format/format.hpp"
#include "5-collections/trace/report.hpp"
#include "6-resources/dl/host_symbol.hpp"
#include "6-resources/dl/library.hpp"

static int notes[32];
static ca::usz noted = 0U;

static auto note(const int value) -> void {
	if (noted < sizeof(notes) / sizeof(notes[0]))
		notes[noted++] = value;
}

static const ca::dl::host_symbol HOST[] = {{"host_note", reinterpret_cast<void*>(&note)}};

static auto fails_with(const ca::sv::str name, const ca::error expected, const ca::sv::str text, const bool hosted = true) noexcept -> bool {
	auto loaded = hosted ? ca::dl::library::make(name, HOST) : ca::dl::library::make(name);
	if (loaded)
		return false;
	ca::report<>& report = loaded.error();
	auto printed = ca::format("{}", report);
	const bool matches = report.error() == expected && printed && printed->view().contains(text);
	if (printed)
		static_cast<void>(printed->cleanup());
	return report.cleanup() && matches;
}

static auto use(void) noexcept -> int {
	auto user = ca::dl::library::make(CA_TEST_LIBRARIES "/libuser.so", HOST);
	if (!user)
		return 1;
	if (noted != 1U || notes[0] != 1)
		return 2;
	if (user->soname() != ca::sv::str{"libuser.so"} || !user->path().ends_with("/libuser.so") || !user->image().segment(ca::elf::segment_types::dynamic))
		return 3;
	const auto sum = user->find_as<int(int)>("user_sum");
	if (!sum || (*sum)(5) != 5 + 42 + 7 + 100 + 1000 || noted != 2U || notes[1] != 15)
		return 4;
	const auto name = user->find_as<const char*(int)>("user_name");
	if (!name || ca::sv::str{(*name)(8)} != ca::sv::str{"eight"} || ca::sv::str{(*name)(1)} != ca::sv::str{"one"})
		return 5;
	const auto value = user->find_as<int(void)>("base_value");
	const auto fast = user->find_as<int(void)>("base_fast");
	const auto local_fast = user->find_as<int(void)>("base_local_fast");
	if (!value || (*value)() != 42 || !fast || (*fast)() != 7 || !local_fast || (*local_fast)() != 8)
		return 6;
	if (user->find("no_such_symbol") || user->find("host_note") || user->find("twice"))
		return 7;

	auto base = ca::dl::library::make(CA_TEST_LIBRARIES "/libbase.so");
	auto again = ca::dl::library::make("libbase.so");
	if (!base || !again || noted != 2U)
		return 8;
	const auto shared = base->find("base_value");
	if (!shared || *shared != reinterpret_cast<void*>(*value) || again->path() != base->path())
		return 9;
	const auto counter = base->find_as<int>("base_counter");
	if (!counter || **counter != 100 || base->find("user_sum"))
		return 10;

	if (!user->cleanup() || user->is_open() || user->find("user_sum") || noted != 2U)
		return 11;
	if (!again->cleanup() || noted != 2U || (*value)() != 42)
		return 12;
	if (!base->cleanup() || noted != 3U || notes[2] != 2)
		return 13;

	auto reloaded = ca::dl::library::make(CA_TEST_LIBRARIES "/libuser.so", HOST);
	if (!reloaded || noted != 4U || notes[3] != 1)
		return 14;
	const auto resum = reloaded->find_as<int(int)>("user_sum");
	if (!resum || (*resum)(1) != 1 + 42 + 7 + 100 + 1000)
		return 15;
	ca::dl::library moved{static_cast<ca::dl::library&&>(*reloaded)};
	if (reloaded->is_open() || !moved.is_open() || !reloaded->cleanup())
		return 16;
	if (!moved.cleanup() || noted != 6U || notes[5] != 2)
		return 17;
	return 0;
}

auto main(void) -> int {
	const int used = use();
	if (used != 0)
		return used;
	if (!fails_with(CA_TEST_LIBRARIES "/libbroken.so", ca::errc::no_entry, "undefined symbol host_missing, needed by "))
		return 20;
	if (!fails_with(CA_TEST_LIBRARIES "/libuser.so", ca::errc::no_entry, "undefined symbol host_note", false))
		return 21;
	if (!fails_with(CA_TEST_LIBRARIES "/libnothing.so", ca::errc::no_entry, "libnothing.so: cannot open"))
		return 22;
	if (!fails_with("libca_test_nothing.so", ca::errc::no_entry, "libca_test_nothing.so: not found"))
		return 23;
	if (!fails_with("/proc/self/status", ca::errc::not_supported, "not an ELF64 x86-64 shared object"))
		return 24;
	if (!fails_with(CA_TEST_LIBRARIES "/liborphan.so", ca::errc::no_entry, "libca_ghost.so: not found"))
		return 28;
	if (!fails_with(CA_TEST_LIBRARIES "/liborphan.so", ca::errc::no_entry, "/liborphan.so"))
		return 29;
	if (noted != 6U)
		return 25;
	auto user = ca::dl::library::make(CA_TEST_LIBRARIES "/libuser.so", HOST);
	if (!user || noted != 7U)
		return 26;
	return user->cleanup() && noted == 8U ? 0 : 27;
}
