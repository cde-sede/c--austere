// expect: exit 0
#include "5-collections/regex/regex.hpp"
#include "5-collections/string/str.hpp"

using kind = ca::regex_error::kind;

static auto rejected(const ca::sv::str pattern, const kind expected) noexcept -> bool {

	const ca::optional<ca::regex_error> problem = ca::regex_pattern{pattern}.check();
	if (!problem || problem->what() != expected || problem->message().empty())
		return false;
	auto made = ca::regex::make(ca::regex_pattern{pattern});
	return !made && made.error() == ca::errc::invalid_argument;
}

auto main(void) -> int {

	if (!rejected("(a)\\1", kind::backreference) || !rejected("(?P<x>a)(?P=x)", kind::backreference))
		return 1;
	if (!rejected("a(?=b)", kind::lookaround) || !rejected("a(?!b)", kind::lookaround) || !rejected("(?<=a)b", kind::lookaround) || !rejected("(?<!a)b", kind::lookaround))
		return 2;
	if (!rejected("(a)?(?(1)b|c)", kind::conditional) || !rejected("(?>a)", kind::atomic) || !rejected("a++", kind::atomic))
		return 3;
	if (!rejected("\\N{DIGIT ONE}", kind::named_character) || !rejected("(?L)a", kind::bad_flags) || !rejected("(?au)a", kind::ascii_and_unicode))
		return 4;
	if (!rejected("\\uD800", kind::bad_code_point) || !rejected("\\U00110000", kind::bad_code_point) || !rejected("[\\777]", kind::bad_escape))
		return 5;
	if (!rejected(ca::sv::str{"\xff", 1U}, kind::invalid_utf8) || !rejected("a{100001}", kind::repeat_too_large) || !rejected("(?#x", kind::unterminated_comment))
		return 6;
	auto deep = ca::str::make("");
	if (!deep)
		return 7;
	for (int i = 0; i < 200; ++i) {
		if (!deep->push('('))
			return 8;
	}
	if (!rejected(*deep, kind::too_deep))
		return 9;
	auto huge = ca::regex::make("(?:a{1000}){1000}");
	if (huge || huge.error() != ca::errc::invalid_argument)
		return 10;
	auto ascii = ca::regex::make("(?u)a", ca::regex_flags::ascii);
	if (ascii || ascii.error() != ca::errc::invalid_argument)
		return 11;
	return deep->cleanup() ? 0 : 12;
}
