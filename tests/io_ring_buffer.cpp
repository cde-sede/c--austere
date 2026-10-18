// expect: exit 0
#include "2-vocabulary/io/is_buffered_reader.hpp"
#include "2-vocabulary/io/is_stream.hpp"
#include "2-vocabulary/io/is_windowed_writer.hpp"
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/io/read_line.hpp"
#include "5-collections/io/read_to_end.hpp"
#include "5-collections/io/ring_buffer.hpp"

static_assert(ca::io::is_stream<ca::io::ring_buffer<>>);
static_assert(ca::io::is_buffered_reader<ca::io::ring_buffer<>>);
static_assert(ca::io::is_windowed_writer<ca::io::ring_buffer<>>);

static auto drained(ca::io::ring_buffer<>& ring, const ca::sv::str expected) noexcept -> bool {
	char out[64];
	auto got = ring.read(out);
	return got && ca::sv::str{out, *got} == expected;
}

auto main(void) -> int {
	if (ca::io::ring_buffer<>::make(0U))
		return 1;
	auto made = ca::io::ring_buffer<>::make(12U);
	if (!made || made->capacity() != 16U || !made->empty())
		return 2;
	ca::io::ring_buffer<> ring = ca::move(*made);

	if (!ring.write(ca::sv::str{"0123456789"}))
		return 21;
	char six[6];
	auto got = ring.read(six);
	if (!got || *got != 6U || ring.size() != 4U || ring.space() != 12U)
		return 3;
	if (!ring.write(ca::sv::str{"abcdefghij"}))
		return 24;
	auto first = ring.fill();
	if (!first || *first != ca::sv::str{"6789abcdef"})
		return 4;
	ring.consume(first->size());
	auto rest = ring.fill();
	if (!rest || *rest != ca::sv::str{"ghij"} || !drained(ring, "ghij") || !ring.empty())
		return 5;

	if (!ring.write(ca::sv::str{"0123456789ab"}) || !drained(ring, "0123456789ab"))
		return 6;
	if (!ring.write(ca::sv::str{"0123456789"}))
		return 22;
	ring.consume(8U);
	auto formatted = ca::format_to(ring, "{}-{}", 12345, "xyz!");
	if (!formatted || *formatted != 10U || !ring.flush() || !drained(ring, "8912345-xyz!"))
		return 7;

	auto fits = ring.write(ca::sv::str{"0123456789"});
	auto too_long = ring.write(ca::sv::str{"too long"});
	auto after = ring.write(ca::sv::str{"ok"});
	if (!fits || too_long || too_long.error() != ca::errc::no_space || after || ring.error() != ca::errc::no_space || ring.size() != 10U)
		return 8;
	auto flushed = ring.flush();
	if (flushed || !ring.error().ok())
		return 9;
	ring.consume(4U);
	auto dropped = ca::format_to(ring, "{}", "a message longer than the space left");
	if (dropped || ring.size() != 6U || ring.flush())
		return 10;
	if (!ring.write(ca::sv::str{"\nnext\n"}))
		return 23;
	ca::dynamic_array<char> line;
	auto read = ca::io::read_line(ring, line);
	if (!read || !*read || ca::sv::str{line.data(), line.size()} != ca::sv::str{"456789"})
		return 11;
	read = ca::io::read_line(ring, line);
	if (!read || !*read || ca::sv::str{line.data(), line.size()} != ca::sv::str{"next"} || !line.destroy())
		return 12;
	read = ca::io::read_line(ring, line);
	if (!read || *read)
		return 13;

	auto arena_made = ca::arena::make();
	if (!arena_made)
		return 14;
	ca::arena arena = ca::move(*arena_made);
	{
		auto in_arena = ca::make_in<ca::io::ring_buffer<>>(arena, 64U);
		if (!in_arena || in_arena->capacity() != 64U)
			return 15;
		auto both = ca::format_to(*in_arena, "{} and {}", 1, 2);
		ca::dynamic_array<char> all;
		auto count = ca::io::read_to_end(*in_arena, all);
		if (!both || !count || ca::sv::str{all.data(), all.size()} != ca::sv::str{"1 and 2"} || !all.destroy())
			return 16;
		if (!in_arena->destroy())
			return 17;
	}
	if (!arena.destroy())
		return 18;

	if (!ring.destroy() || ring.capacity() != 0U)
		return 19;
	auto refused = ring.write(ca::sv::str{"x"});
	char none[1];
	auto empty = ring.read(none);
	return !refused && !ring.flush() && empty && *empty == 0U ? 0 : 20;
}
