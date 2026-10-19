// expect: exit 0
#include "2-vocabulary/sv/u16.hpp"
#include "2-vocabulary/sv/u32.hpp"
#include "2-vocabulary/unicode/properties_of.hpp"
#include "5-collections/array/collect.hpp"
#include "5-collections/string/inline_str.hpp"
#include "5-collections/string/join.hpp"
#include "5-collections/string/str.hpp"
#include "5-collections/string/u16str.hpp"

static_assert(ca::sv::str{"  a b "}.strip() == ca::sv::str{"a b"});
static_assert(ca::sv::str{"key=value"}.partition("=").after() == ca::sv::str{"value"});
static_assert(ca::sv::str{"a,b,,c"}.count(",") == 3U && ca::sv::str{"x_1"}.is_identifier());
static_assert(ca::unicode::properties_of(U'A').lower().first() == U'a' && ca::unicode::properties_of(U'\x1c').is_space());
static_assert(ca::unicode::properties_of(U'\xe9').is_alpha());

constexpr auto pieces(void) noexcept -> ca::usz {

	ca::usz count = 0U;
	for (const ca::sv::str piece : ca::sv::str{" one  two three "}.split())
		count += piece.size();
	return count;
}

static_assert(pieces() == 11U);

static auto join_and_collect(void) noexcept -> int {

	const char* const names[] = {"ann", "bob", "cy"};
	auto joined = ca::join(", ", names);
	if (!joined || *joined != ca::sv::str{"ann, bob, cy"})
		return 1;
	auto fields = ca::collect(ca::sv::str{"a,b,,c"}.split(","));
	if (!fields || fields->size() != 4U || !(*fields)[2].empty() || (*fields)[3] != ca::sv::str{"c"})
		return 2;
	auto back = ca::join(ca::sv::str{"-"}, *fields);
	if (!back || *back != ca::sv::str{"a-b--c"})
		return 3;
	auto lines = ca::collect(ca::sv::str{"x\r\ny\n"}.split_lines());
	if (!lines || lines->size() != 2U || (*lines)[1] != ca::sv::str{"y"})
		return 4;
	auto empty = ca::join(", ", ca::sv::str{""}.split());
	if (!empty || !empty->empty())
		return 5;
	return joined->cleanup() && fields->cleanup() && back->cleanup() && lines->cleanup() && empty->cleanup() ? 0 : 6;
}

static auto wide(void) noexcept -> int {

	auto w = ca::u16str::from(u"  Hello, wide  ");
	if (!w || w->strip() != ca::sv::u16{u"Hello, wide"} || !w->contains(u"wide"))
		return 10;
	auto upper = w->upper();
	if (!upper || *upper != ca::sv::u16{u"  HELLO, WIDE  "})
		return 11;
	auto from_utf8 = ca::u16str::from(ca::sv::str{"caf\xc3\xa9 \xf0\x9f\x98\x80"});
	if (!from_utf8 || *from_utf8 != ca::sv::u16{u"caf\xe9 \xd83d\xde00"} || from_utf8->code_point_count() != 6U)
		return 12;
	auto to_utf8 = ca::str::from(ca::sv::u32{U"\x1F600!"});
	if (!to_utf8 || *to_utf8 != ca::sv::str{"\xf0\x9f\x98\x80!"})
		return 13;
	return w->cleanup() && upper->cleanup() && from_utf8->cleanup() && to_utf8->cleanup() ? 0 : 14;
}

static auto multibyte(void) noexcept -> int {

	auto s = ca::str::from("caf\xc3\xa9  ");
	if (!s || s->strip_end() != ca::sv::str{"caf\xc3\xa9"})
		return 20;
	auto upper = s->upper();
	if (!upper || *upper != ca::sv::str{"CAF\xc3\x89  "})
		return 21;
	if (ca::sv::str{"\xc3\xa9x\xc3\xa9"}.strip("\xc3\xa9") != ca::sv::str{"x"})
		return 22;
	auto centered = ca::str::from("\xc3\xa9");
	if (!centered)
		return 23;
	auto wide_center = centered->center(3U, U'\x2014');
	if (!wide_center || *wide_center != ca::sv::str{"\xe2\x80\x94\xc3\xa9\xe2\x80\x94"})
		return 24;
	auto bad = ca::str::from("a\xffz");
	if (!bad)
		return 25;
	auto bad_upper = bad->upper();
	if (!bad_upper || *bad_upper != ca::sv::str{"A\xffZ"})
		return 26;
	return s->cleanup() && upper->cleanup() && centered->cleanup() && wide_center->cleanup() && bad->cleanup() && bad_upper->cleanup() ? 0 : 27;
}

static auto fixed(void) noexcept -> int {

	auto tag = ca::inline_str<16>::from(" x,y ");
	if (!tag || tag->strip() != ca::sv::str{"x,y"} || tag->count(",") != 1U || tag->is_alpha())
		return 30;
	ca::usz parts = 0U;
	for (const ca::sv::str piece : tag->strip().split(","))
		parts += piece.size();
	return parts == 2U ? 0 : 31;
}

auto main(void) -> int {

	if (const int failed = join_and_collect(); failed != 0)
		return failed;
	if (const int failed = wide(); failed != 0)
		return failed;
	if (const int failed = multibyte(); failed != 0)
		return failed;
	return fixed();
}
