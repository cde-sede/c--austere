// expect: exit 0
// link: both
#include "3-system/file/read.hpp"
#include "3-system/fs/options.hpp"
#include "3-system/io/pipe.hpp"
#include "6-resources/io/writer.hpp"
#include "6-resources/print/print.hpp"

static auto drained(const int fd, const ca::sv::str expected) noexcept -> bool {

	char got[128];
	auto size = ca::sys::read(fd, got);
	if (expected.empty())
		return !size;
	return size && ca::sv::str{got, *size} == expected;
}

auto main(void) -> int {
	auto ends = ca::sys::pipe(ca::fs::nonblock_t::OPEN_VALUE);
	if (!ends)
		return 1;
	const int in = ends->read_end();
	char buffer[16];
	ca::io::writer full{ends->write_end(), ca::io::buffering::full, buffer};
	auto line = ca::print(full, "a{}\n", 1);
	if (!line || *line != 3U || !drained(in, "") || full.pending() != 3U || !full.flush() || !drained(in, "a1\n"))
		return 2;
	auto spill = ca::print(full, "{}", "0123456789abcdef-tail");
	if (!spill || *spill != 21U || !drained(in, "0123456789abcdef") || full.pending() != 5U || !full.flush() || !drained(in, "-tail"))
		return 3;
	if (!full.write(ca::sv::str{"this is longer than the buffer"}) || !drained(in, "this is longer than the buffer"))
		return 4;
	if (!full.set_buffering(ca::io::buffering::line) || !ca::print(full, "no newline") || !drained(in, ""))
		return 5;
	if (!ca::print(full, ", then\nmore") || !drained(in, "no newline, then\nmore"))
		return 6;
	auto direct = ca::io::writer::unbuffered(ends->write_end());
	auto staged = ca::print(direct, "x={} y={}\n", 1, 2);
	if (!staged || *staged != 8U || !drained(in, "x=1 y=2\n"))
		return 7;
	return full.flush() && direct.flush() ? 0 : 8;
}
