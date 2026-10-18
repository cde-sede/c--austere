// expect: exit 0
#include "3-system/error/errc.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/format/format.hpp"
#include "5-collections/format/format_append.hpp"
#include "5-collections/format/printf_to.hpp"
#include "5-collections/io/memory_reader.hpp"
#include "5-collections/io/read_line.hpp"
#include "5-collections/io/read_to_end.hpp"
#include "5-collections/io/string_writer.hpp"
#include "5-collections/string/inline_str.hpp"
#include "5-collections/string/str.hpp"

static_assert(ca::io::is_windowed_writer<ca::io::string_writer<ca::str>>);
static_assert(ca::io::is_windowed_writer<ca::io::string_writer<ca::inline_str<8>>>);

static auto formats(void) noexcept -> int {

	auto s = ca::format("{}-{:04} {}", "id", 42, u"\u00e9");
	if (!s || *s != ca::sv::str{"id-0042 \u00e9"} || s->capacity() != ca::str::INLINE_CAPACITY)
		return 1;
	if (!ca::format_append(*s, " [{:>3}]", 7) || *s != ca::sv::str{"id-0042 \u00e9 [  7]"})
		return 2;
	auto big = ca::format("{:>3000}|{}", "x", *s);
	if (!big || big->size() != 3001U + s->size() || (*big)[2999] != 'x' || big->c_str()[big->size()] != '\0')
		return 3;
	ca::io::string_writer<ca::str> writer{*big};
	for (int i = 0; i < 1000; ++i)
		if (!ca::printf_to(writer, "%d,", i))
			return 6;
	if (!writer.flush() || !big->view().ends_with("998,999,"))
		return 4;
	return s->destroy() && big->destroy() ? 0 : 5;
}

static auto bounded(void) noexcept -> int {

	ca::inline_str<8> tag;
	if (!ca::format_append(tag, "{}", 1234) || tag != ca::sv::str{"1234"})
		return 10;
	auto over = ca::format_append(tag, "{}", 123456789);
	if (over || over.error() != ca::errc::no_space || tag != ca::sv::str{"1234"})
		return 11;
	ca::io::string_writer<ca::inline_str<8>> writer{tag};
	auto cut = ca::format_to(writer, "{}", "abcdefgh");
	if (cut || cut.error() != ca::errc::no_space || tag != ca::sv::str{"1234"})
		return 12;
	auto flushed = writer.flush();
	if (flushed || flushed.error() != ca::errc::no_space || !writer.flush())
		return 13;
	auto fits = ca::format_to(writer, "{}", "abcd");
	return fits && *fits == 4U && tag == ca::sv::str{"1234abcd"} ? 0 : 14;
}

static auto in_arena(void) noexcept -> int {

	auto made = ca::arena::make();
	if (!made)
		return 20;
	ca::arena arena = ca::move(*made);
	auto s = ca::format(arena.allocator(), "{} {} {}", "allocated", "in the", "arena");
	if (!s || *s != ca::sv::str{"allocated in the arena"})
		return 21;
	return s->destroy() && arena.destroy() ? 0 : 22;
}

static auto lines(void) noexcept -> int {

	ca::io::memory_reader reader{"first line\nsecond line, longer than twenty-three\nend"};
	ca::str line;
	auto got = ca::io::read_line(reader, line);
	if (!got || !*got || line != ca::sv::str{"first line"})
		return 30;
	ca::inline_str<8> small;
	got = ca::io::read_line(reader, small);
	if (got || got.error() != ca::errc::no_space)
		return 31;
	ca::str rest;
	auto all = ca::io::read_to_end(reader, rest);
	if (!all || rest.view().ends_with("end") == false)
		return 32;
	return line.destroy() && rest.destroy() ? 0 : 33;
}

auto main(void) -> int {

	if (const int failed = formats(); failed != 0)
		return failed;
	if (const int failed = bounded(); failed != 0)
		return failed;
	if (const int failed = in_arena(); failed != 0)
		return failed;
	return lines();
}
