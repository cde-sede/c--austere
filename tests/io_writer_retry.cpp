// expect: exit 0
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/read.hpp"
#include "3-system/file/write.hpp"
#include "3-system/fs/options.hpp"
#include "3-system/io/pipe.hpp"
#include "6-resources/io/writer.hpp"
#include "6-resources/print/print.hpp"

static auto empty_pipe(const int fd) noexcept -> ca::usz {

	char sink[4096];
	ca::usz total = 0U;
	for (;;) {
		auto got = ca::sys::read(fd, sink);
		if (!got || *got == 0U)
			return total;
		total += *got;
	}
}

auto main(void) -> int {
	auto ends = ca::sys::pipe(ca::fs::nonblock_t::OPEN_VALUE);
	if (!ends)
		return 1;
	const int in = ends->read_end();
	const char block[4096] = {};
	for (;;) {
		auto filled = ca::sys::write(ends->write_end(), ca::span{block, sizeof(block)});
		if (!filled)
			break;
	}

	char buffer[16];
	ca::io::writer w{ends->write_end(), ca::io::buffering::full, buffer};
	auto held = ca::print(w, "abc\n");
	if (!held || *held != 4U || w.pending() != 4U)
		return 2;
	auto busy = w.flush();
	if (busy || busy.error() != ca::errc::try_again || w.pending() != 4U || !w.error().ok())
		return 3;

	auto cut = ca::print(w, "{}", "0123456789abcdefXYZ");
	if (cut || cut.error() != ca::errc::try_again || w.pending() != 4U || w.error() != ca::errc::try_again)
		return 4;
	w.clear_error();

	if (empty_pipe(in) == 0U || !w.flush() || w.pending() != 0U)
		return 5;
	char got[16];
	auto kept = ca::sys::read(in, got);
	if (!kept || ca::sv::str{got, *kept} != ca::sv::str{"abc\n"})
		return 6;

	if (!ca::print(w, "zz") || w.pending() != 2U)
		return 7;
	w.discard();
	auto nothing = ca::sys::read(in, got);
	if (!w.flush() || nothing)
		return 8;
	return ca::sys::close(in) && ca::sys::close(ends->write_end()) ? 0 : 9;
}
