// expect: exit 0
#include "2-vocabulary/sv/str.hpp"
#include "2-vocabulary/sv/u16.hpp"
#include "2-vocabulary/sv/u32.hpp"
#include "2-vocabulary/sv/u8.hpp"
#include "2-vocabulary/sv/wide.hpp"
#include "2-vocabulary/unicode/code_points.hpp"
#include "2-vocabulary/unicode/count_code_points.hpp"
#include "2-vocabulary/unicode/decode.hpp"
#include "2-vocabulary/unicode/encode.hpp"
#include "2-vocabulary/unicode/find_invalid.hpp"
#include "2-vocabulary/unicode/is_valid.hpp"
#include "2-vocabulary/unicode/transcode.hpp"
#include "2-vocabulary/unicode/transcoded_size.hpp"

static_assert(ca::unicode::is_valid(ca::sv::u8{u8"h\u00E9llo \U0001F600"}));
static_assert(ca::unicode::count_code_points(ca::sv::u8{u8"h\u00E9llo \U0001F600"}) == 7U);
static_assert(ca::unicode::count_code_points(ca::sv::u16{u"h\u00E9llo \U0001F600"}) == 7U);
constexpr auto first(const ca::sv::str text, const char32_t point, const unsigned length) noexcept -> bool {

	const auto decoded = ca::unicode::decode(text);
	return decoded && decoded->code_point() == point && decoded->length() == length;
}

static_assert(first(ca::sv::str{"\xE2\x82\xAC"}, U'\u20AC', 3U));
static_assert(!ca::unicode::decode(ca::sv::str{}).has_value());
static_assert(ca::unicode::find_invalid(ca::sv::str{"ab\xC0\x80"}) == 2U);
static_assert(ca::unicode::find_invalid(ca::sv::str{"\xED\xA0\x80"}) == 0U);
static_assert(ca::unicode::find_invalid(ca::sv::str{"\xF4\x90\x80\x80"}) == 0U);
static_assert(ca::unicode::find_invalid(ca::sv::str{"\xE2\x82"}) == 0U);
static_assert(first(ca::sv::str{"\xE2\x82x"}, U'\uFFFD', 2U));
static_assert(first(ca::sv::str{"\xF0\x80"}, U'\uFFFD', 1U));
static_assert(ca::unicode::transcoded_size<char16_t>(ca::sv::str{"a\U0001F600"}) == 3U);
static_assert(ca::unicode::transcoded_size<char>(ca::sv::u32{U"a\u00E9\U0001F600"}) == 7U);
static_assert(!ca::unicode::is_valid(ca::sv::u16{u"a\xD800" u"b"}));
static_assert(!ca::unicode::is_valid(ca::sv::u32{U"\x110000"}));

constexpr auto roundtrip(void) noexcept -> bool {

	char16_t wide[8]{};
	const auto there = ca::unicode::transcode(ca::sv::str{"\xC3\xA9\xF0\x9F\x98\x80!"}, wide, 8U);
	char back[16]{};
	const auto again = ca::unicode::transcode(ca::sv::u16{wide, there.written()}, back, 16U);
	return there.read() == 7U && there.written() == 4U && again.written() == 7U && ca::sv::str{back, again.written()} == ca::sv::str{"\xC3\xA9\xF0\x9F\x98\x80!"};
}

static_assert(roundtrip());

auto main(void) -> int {
	char long_text[100];
	for (char& c : long_text)
		c = 'a';
	long_text[70] = '\xFF';
	if (ca::unicode::find_invalid(ca::sv::str{long_text, 100U}) != 70U)
		return 1;
	if (ca::unicode::count_code_points(ca::sv::str{long_text, 100U}) != 100U)
		return 2;
	char32_t points[4]{};
	ca::usz n = 0U;
	for (const char32_t c : ca::unicode::code_points{"a\xFF\xE2\x82\xAC"})
		points[n++] = c;
	if (n != 3U || points[0] != U'a' || points[1] != U'\uFFFD' || points[2] != U'\u20AC')
		return 3;
	char tight[3];
	const auto cut = ca::unicode::transcode(ca::sv::u32{U"a\u20AC"}, tight, 3U);
	if (cut.read() != 1U || cut.written() != 1U)
		return 4;
	char sanitized[8];
	const auto clean = ca::unicode::transcode(ca::sv::str{"x\xC0y"}, sanitized, 8U);
	if (ca::sv::str{sanitized, clean.written()} != ca::sv::str{"x\xEF\xBF\xBDy"})
		return 5;
	wchar_t w[2];
	if (ca::unicode::encode(U'\U0001F600', w) != (sizeof(wchar_t) == 4U ? 1U : 2U))
		return 6;
	if (ca::sv::wide{L"abc"}.find(ca::sv::wide{L"bc"}) != 1U || ca::sv::str{"  x \n"}.strip() != ca::sv::str{"x"})
		return 7;
	if (ca::sv::str{"abc"}.compare(ca::sv::str{"abd"}) >= 0 || ca::sv::u16{u"b"}.compare(ca::sv::u16{u"ab"}) <= 0)
		return 8;
	return ca::sv::str{"a.b.c"}.find_last(ca::sv::str{"."}) == 3U ? 0 : 9;
}
