// expect: exit 0
#include "3-system/file/read.hpp"
#include "3-system/fs/options.hpp"
#include "3-system/io/pipe.hpp"
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/format/format_to.hpp"
#include "6-resources/fs/descriptor.hpp"
#include "6-resources/io/buffered_writer.hpp"

static auto drained(const int fd, const ca::sv::str expected) noexcept -> bool {

	char got[256];
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
	auto borrowed = ca::io::buffered_writer<>::over(ends->write_end(), ca::io::buffering::full, 8U);
	if (!borrowed || borrowed->capacity() != 8U || borrowed->owns_descriptor())
		return 2;
	auto pair = ca::format_to(*borrowed, "{}-{}", 12, 34);
	if (!pair || *pair != 5U || !drained(in, "") || borrowed->pending() != 5U)
		return 3;
	if (!borrowed->resize(128U) || borrowed->capacity() != 128U || !drained(in, "12-34"))
		return 4;
	auto longer = ca::format_to(*borrowed, "{}", "longer than eight bytes");
	if (!longer || !drained(in, "") || !borrowed->resize(0U) || !drained(in, "longer than eight bytes"))
		return 5;
	auto direct = ca::format_to(*borrowed, "{}", "direct");
	if (!direct || !drained(in, "direct") || !borrowed->cleanup())
		return 6;

	auto made = ca::arena::make();
	if (!made)
		return 7;
	ca::arena arena = ca::move(*made);
	{
		auto owner = ca::make_in<ca::io::buffered_writer<>>(arena, ca::fs::descriptor::adopt(ends->write_end()), ca::io::buffering::line, 32U);
		if (!owner || !owner->owns_descriptor())
			return 8;
		auto x = ca::format_to(*owner, "x={}", 1);
		if (!x || !drained(in, ""))
			return 9;
		auto y = ca::format_to(owner->writer(), " y={}\n", 2);
		if (!y || !drained(in, "x=1 y=2\n") || !owner->cleanup() || owner->native() != -1)
			return 10;
	}
	char probe = 0;
	auto closed = ca::sys::read(in, ca::span{&probe, 1U});
	return closed && *closed == 0U && arena.cleanup() ? 0 : 11;
}
