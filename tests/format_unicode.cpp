// expect: exit 0
#include "2-vocabulary/sv/u16.hpp"
#include "2-vocabulary/sv/u32.hpp"
#include "2-vocabulary/sv/u8.hpp"
#include "2-vocabulary/sv/wide.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/format/printf_to.hpp"
#include "5-collections/io/memory_writer.hpp"
#include "5-collections/string/u16str.hpp"

static auto check(ca::io::memory_writer& m, const ca::sv::str expected) noexcept -> bool {

	const bool same = m.view() == expected;
	m.clear();
	return same;
}

auto main(void) -> int {
	char buffer[512];
	ca::io::memory_writer m{buffer};
	const char16_t array[8] = u"arr";
	const char16_t* none = nullptr;
	auto wide = ca::format_to(m, "{} {} {} {} {} {}", u"\u00e9t\u00e9", U"\U0001F600", L"wide", ca::sv::u16{u"view"}, array, none);
	if (!wide || *wide != m.size() || !check(m, "\u00e9t\u00e9 \U0001F600 wide view arr (null)"))
		return 1;
	if (!ca::format_to(m, "{} {} {} {d} {x} {c}", u8"utf8", char8_t{'x'}, U'\u20ac', U'\u20ac', u'\u00e9', L'w') || !check(m, "utf8 x \u20ac 8364 e9 w"))
		return 2;
	if (!ca::format_to(m, "[{:>4}] [{:.2}] [{:^5}] [{:<3}]", "\u00e9t\u00e9", "\u00e9t\u00e9", u"\u00e9", U'\u20ac')
	 || !check(m, "[ \u00e9t\u00e9] [\u00e9t] [  \u00e9  ] [\u20ac  ]"))
		return 3;
	if (!ca::format_to(m, "{} {} {}", u"a\xD800" u"b", u'\xDC00', U'\x110000') || !check(m, "a\ufffdb \ufffd \ufffd"))
		return 4;
	if (!ca::format_to(m, "{?} {?} {?} {?}", "tab\there \"q\" \\ \x01 \xFF \u00e9", 'x', '\'', '\n')
	 || !check(m, "\"tab\\there \\\"q\\\" \\\\ \\u{1} \\x{ff} \u00e9\" 'x' '\\'' '\\n'"))
		return 5;
	if (!ca::format_to(m, "{?} {?} {?} [{:>6?}] [{:.1?}]", u"a\xD800", U'\x110000', U'\u20ac', "ab", "xyz")
	 || !check(m, "\"a\\u{d800}\" '\\u{110000}' '\u20ac' [  \"ab\"] [\"x\"]"))
		return 6;
	if (!ca::printf_to(m, "%ls %lc %s %5.1ls|", L"wide", L'\u00e9', u"u16", L"\u00e9t\u00e9") || !check(m, "wide \u00e9 u16     \u00e9|"))
		return 7;
	auto s = ca::u16str::from(u"owned");
	if (!s)
		return 8;
	auto shown = ca::format_to(m, "{} {p}", *s, ca::sv::u16{});
	const bool pointer = shown && m.view().starts_with(ca::sv::str{"owned 0x"});
	m.clear();
	if (!pointer || !s->cleanup())
		return 9;
	char small[6];
	ca::io::memory_writer cut{small};
	auto dropped = ca::format_to(cut, "{}", U"\u00e9\u00e9\u00e9\u00e9");
	return !dropped && cut.view().empty() && !cut.flush() ? 0 : 10;
}
