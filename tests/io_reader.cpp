// expect: exit 0
#include "3-system/file/close.hpp"
#include "3-system/file/write.hpp"
#include "3-system/io/pipe.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/io/read_line.hpp"
#include "6-resources/io/reader.hpp"

static_assert(ca::io::is_buffered_reader<ca::io::reader>);

static auto text(const ca::result<ca::sv::str>& got, const ca::sv::str expected) noexcept -> bool {
	return got && *got == expected;
}

auto main(void) -> int {
	auto ends = ca::sys::pipe(0);
	if (!ends)
		return 1;
	const ca::sv::str input{"ab\ncdefghijkl\nmnop\nqrstuvwxyz0123456789 a line longer than sixteen\nend"};
	if (!ca::sys::write(ends->write_end(), input) || !ca::sys::close(ends->write_end()))
		return 2;
	char buffer[16];
	ca::io::reader r{ends->read_end(), buffer};
	if (!text(r.read_until('\n'), "ab\n") || !text(r.read_until('\n'), "cdefghijkl\n"))
		return 3;
	char four[4];
	auto got = r.read(four);
	if (!got || *got != 2U || four[1] != 'n' || r.available() != 0U)
		return 4;
	if (!text(r.read_until('\n'), "op\n"))
		return 5;
	if (!text(r.read_until('\n'), "qrstuvwxyz012345") || !text(r.read_until(' '), "6789 "))
		return 6;
	char bigger[64];
	if (!r.set_buffer(bigger) || r.capacity() != 64U)
		return 7;
	ca::dynamic_array<char> line;
	auto read = ca::io::read_line(r, line);
	if (!read || !*read || ca::sv::str{line.data(), line.size()} != ca::sv::str{"a line longer than sixteen"})
		return 8;
	char tiny[2];
	if (r.set_buffer(tiny) || r.available() == 0U)
		return 9;
	if (!text(r.read_until('\n'), "end") || !text(r.read_until('\n'), ""))
		return 10;
	auto view = r.fill();
	auto unbuffered = ca::io::reader::unbuffered(ends->read_end());
	return view && view->empty() && !unbuffered.fill() && line.cleanup() && ca::sys::close(ends->read_end()) ? 0 : 11;
}
