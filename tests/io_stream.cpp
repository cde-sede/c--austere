// expect: exit 0
#include "2-vocabulary/io/is_buffered_reader.hpp"
#include "2-vocabulary/io/is_stream.hpp"
#include "2-vocabulary/io/is_windowed_writer.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/read.hpp"
#include "3-system/file/write.hpp"
#include "3-system/io/pipe.hpp"
#include "5-collections/format/format_to.hpp"
#include "6-resources/io/stream.hpp"

static_assert(ca::io::is_stream<ca::io::stream>);
static_assert(ca::io::is_buffered_reader<ca::io::stream>);
static_assert(ca::io::is_windowed_writer<ca::io::stream>);

static auto received(const int fd, const ca::sv::str expected) noexcept -> bool {
	char out[64];
	auto got = ca::sys::read(fd, out);
	return got && ca::sv::str{out, *got} == expected;
}

auto main(void) -> int {
	auto to_stream = ca::sys::pipe(0);
	auto from_stream = ca::sys::pipe(0);
	if (!to_stream || !from_stream)
		return 1;
	char input[16];
	char output[32];
	ca::io::stream s{to_stream->read_end(), from_stream->write_end(), ca::io::buffering::full, input, output};
	if (s.input() != to_stream->read_end() || s.output() != from_stream->write_end())
		return 2;

	auto ping = ca::format_to(s, "ping {}\n", 1);
	if (!ping || *ping != 7U || s.pending() != 7U)
		return 3;
	if (!ca::sys::write(to_stream->write_end(), ca::sv::str{"pong 1\npong 2\n"}))
		return 4;
	auto answer = s.read_until('\n');
	if (!answer || *answer != ca::sv::str{"pong 1\n"} || s.pending() != 0U || !received(from_stream->read_end(), "ping 1\n"))
		return 5;

	ca::io::stream moved{ca::move(s)};
	if (moved.reader().tied() != &moved.writer() || s.input() != -1)
		return 6;
	if (!moved.write(ca::sv::str{"ping 2\n"}))
		return 12;
	auto buffered = moved.fill();
	if (!buffered || *buffered != ca::sv::str{"pong 2\n"} || moved.pending() != 7U)
		return 7;
	moved.consume(buffered->size());
	if (!ca::sys::close(to_stream->write_end()))
		return 8;
	char rest[4];
	auto end = moved.read(rest);
	if (!end || *end != 0U || !received(from_stream->read_end(), "ping 2\n"))
		return 9;

	if (!moved.write(ca::sv::str{"bye\n"}) || !moved.flush() || !received(from_stream->read_end(), "bye\n"))
		return 10;
	ca::io::stream closed;
	auto refused = closed.write(ca::sv::str{"x"});
	return !refused && !closed.flush() && ca::sys::close(to_stream->read_end()) && ca::sys::close(from_stream->read_end()) && ca::sys::close(from_stream->write_end()) ? 0 : 11;
}
