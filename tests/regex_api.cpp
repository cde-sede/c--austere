// expect: exit 0
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/regex/regex.hpp"
#include "5-collections/string/str.hpp"

static constexpr auto problem(const char* const pattern) noexcept -> ca::regex_error {

	const ca::optional<ca::regex_error> found = ca::regex_pattern{ca::sv::str{pattern}}.check();
	return found ? *found : ca::regex_error{ca::regex_error::kind::too_large, 0U};
}

static_assert(!ca::regex_pattern{"(?P<year>\\d{4})-(\\d\\d)"}.check());
static_assert(problem("a(b").what() == ca::regex_error::kind::missing_paren);
static_assert(problem("ab)").position() == 2U);
static_assert(!ca::regex_pattern{ca::sv::str{"a # ["}}.check(ca::regex_flags::verbose));
static_assert((ca::regex_flags::ignore_case | ca::regex_flags::multiline) != ca::regex_flags::none);

static auto upper_first(const ca::regex_match<>& match) noexcept -> ca::sv::str {
	return match[1U] == ca::sv::str{"cat"} ? ca::sv::str{"Cat"} : match.group();
}

static auto groups(void) noexcept -> int {

	auto made = ca::regex::compile("(?P<year>\\d{4})-(?P<month>\\d\\d)(-(\\d\\d))?");
	if (!made)
		return 1;
	ca::regex& regex = *made;
	if (regex.group_count() != 4U || regex.group_index("month") != ca::optional<ca::usz>{2U} || regex.group_index("day"))
		return 2;
	auto found = regex.search("due 2026-11, paid 2026-12-03");
	if (!found || !*found)
		return 3;
	ca::regex_match<>& match = **found;
	if (match.group() != ca::sv::str{"2026-11"} || match["year"] != ca::sv::str{"2026"} || match[2U] != ca::sv::str{"11"})
		return 4;
	if (match.group(3U) || match.start(4U) || match[4U] != ca::sv::str{} || match.start() != 4U || match.end() != 11U)
		return 5;
	if (match.group("month") != ca::optional<ca::sv::str>{ca::sv::str{"11"}} || match.group("nope") || match.group(9U))
		return 6;
	auto expanded = match.expand("\\g<month>/\\g<year>");
	if (!expanded || *expanded != ca::sv::str{"11/2026"})
		return 7;
	auto copy = match.clone();
	if (!copy || copy->group(2U) != match.group(2U))
		return 8;
	auto full = regex.fullmatch("2026-12-03");
	if (!full || !*full || (*full)->group(4U) != ca::optional<ca::sv::str>{ca::sv::str{"03"}})
		return 9;
	auto none = regex.match("x2026-12");
	if (!none || *none)
		return 10;
	return expanded->cleanup() && copy->cleanup() && (*full)->cleanup() && match.cleanup() && regex.cleanup() ? 0 : 11;
}

static auto iteration(void) noexcept -> int {

	auto made = ca::regex::make("\\w+");
	if (!made)
		return 20;
	ca::regex& regex = *made;
	ca::regex::matches_type words = regex.finditer("héllo, wörld");
	ca::usz count = 0U;
	ca::usz bytes = 0U;
	while (true) {
		auto next = words.next();
		if (!next)
			return 21;
		if (!*next)
			break;
		++count;
		bytes += (*next)->group().size();
		if (!(*next)->cleanup())
			return 22;
	}
	if (count != 2U || bytes != 12U)
		return 23;
	auto all = regex.findall("a bb ccc");
	if (!all || all->size() != 3U || (*all)[2U] != ca::sv::str{"ccc"})
		return 24;
	auto pieces = regex.split("a, b, c", 1U);
	if (!pieces)
		return 25;
	return all->cleanup() && pieces->cleanup() && regex.cleanup() ? 0 : 26;
}

static auto substitution(void) noexcept -> int {

	auto made = ca::regex::make("(cat|dog)s?");
	if (!made)
		return 30;
	ca::regex& regex = *made;
	auto plain = regex.sub("[\\1]", "cats and dogs", 1U);
	if (!plain || *plain != ca::sv::str{"[cat] and dogs"})
		return 31;
	auto computed = regex.sub(upper_first, "cats and dogs");
	if (!computed || *computed != ca::sv::str{"Cat and dogs"})
		return 32;
	auto bad = regex.sub("\\9", "cats");
	if (bad || bad.error() != ca::errc::invalid_argument)
		return 33;
	auto quoted = ca::regex::escape("1+1=2? (yes)");
	if (!quoted || *quoted != ca::sv::str{"1\\+1=2\\?\\ \\(yes\\)"})
		return 34;
	auto literal = ca::regex::make(ca::regex_pattern{quoted->view()});
	if (!literal)
		return 35;
	auto found = literal->search("is 1+1=2? (yes)!");
	if (!found || !*found || (*found)->start() != 3U)
		return 36;
	return plain->cleanup() && computed->cleanup() && quoted->cleanup() && (*found)->cleanup() && literal->cleanup() && regex.cleanup() ? 0 : 37;
}

static auto ownership(void) noexcept -> int {

	auto made = ca::regex::make("b+", ca::regex_flags::ignore_case);
	if (!made)
		return 40;
	ca::regex moved = ca::move(*made);
	auto copy = moved.clone();
	if (!copy || copy->pattern() != ca::sv::str{"b+"} || copy->flags() != ca::regex_flags::ignore_case)
		return 41;
	moved = ca::move(*copy);
	auto found = moved.search("aBbB");
	if (!found || !*found || (*found)->group() != ca::sv::str{"BbB"})
		return 42;
	auto arena = ca::arena::make();
	if (!arena)
		return 43;
	auto local = ca::basic_regex<ca::arena_allocator>::make_in(arena->allocator(), "(x)(y)?");
	if (!local)
		return 44;
	auto in_arena = local->search("axyz");
	if (!in_arena || !*in_arena || (*in_arena)->end(2U) != ca::optional<ca::usz>{3U})
		return 45;
	if (!(*in_arena)->cleanup() || !local->cleanup())
		return 46;
	return (*found)->cleanup() && moved.cleanup() && arena->cleanup() ? 0 : 47;
}

static auto linear(void) noexcept -> int {

	auto text = ca::str::make("");
	if (!text)
		return 50;
	for (ca::usz i = 0U; i < 20000U; ++i) {
		if (!text->push('a'))
			return 51;
	}
	auto made = ca::regex::make("(a*)*(a|b)*c");
	if (!made)
		return 52;
	auto found = made->search(*text);
	if (!found || *found)
		return 53;
	return made->cleanup() && text->cleanup() ? 0 : 54;
}

auto main(void) -> int {

	if (const int failed = groups(); failed != 0)
		return failed;
	if (const int failed = iteration(); failed != 0)
		return failed;
	if (const int failed = substitution(); failed != 0)
		return failed;
	if (const int failed = ownership(); failed != 0)
		return failed;
	return linear();
}
