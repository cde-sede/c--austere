// Benchmarks for ca::regex; -DUSE_STD runs the same cases on std::regex (ECMAScript, the
// standard library's backtracker). Texts are ASCII, so both see the same characters.

#include "measure.hpp"

static const int TEXT_SIZE = 1 << 18;
static char text[TEXT_SIZE + 1];
static char csv[TEXT_SIZE + 1];
static char letters[4097];

static auto make_text(void) -> void {
	static const char* const words[] = {"the ", "running ", "fox ", "jumps ", "over ", "lazy ", "bob@example.com ", "dogs, ", "singing\n", "42 "};
	int at = 0;
	for (int i = 0; at < TEXT_SIZE - 32; ++i) {
		const char* word = words[(i * 7 + i / 3) % 10];
		while (*word != '\0')
			text[at++] = *word++;
	}
	text[at] = '\0';
	for (int i = 0; i < TEXT_SIZE; ++i)
		csv[i] = i % 9 == 8 ? ',' : static_cast<char>('a' + i % 23);
	csv[TEXT_SIZE] = '\0';
	for (int i = 0; i < 4096; ++i)
		letters[i] = static_cast<char>('a' + i % 3);
	letters[4096] = '\0';
}

static constexpr const char* email = "([a-z]+)@([a-z]+)\\.com";

#ifdef USE_STD
#include <regex>
#include <string>

NI static auto compile(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < 10000; ++i) {
		std::regex r{email};
		sum += r.mark_count();
	}
	sink = sum;
}

NI static auto literal(void) -> void {
	std::regex r{"zebra crossings!"};
	size_t sum = 0;
	for (int i = 0; i < 4; ++i)
		sum += std::regex_search(text, r) ? 1U : 0U;
	sink = sum;
}

NI static auto words(void) -> void {
	std::regex r{"\\w+"};
	size_t sum = 0;
	for (std::cregex_iterator it{text, text + TEXT_SIZE - 32, r}, end; it != end; ++it)
		sum += static_cast<size_t>(it->length());
	sink = sum;
}

NI static auto suffix(void) -> void {
	std::regex r{"[a-z]+ing\\b"};
	size_t sum = 0;
	for (std::cregex_iterator it{text, text + TEXT_SIZE - 32, r}, end; it != end; ++it)
		++sum;
	sink = sum;
}

NI static auto captures(void) -> void {
	std::regex r{email};
	size_t sum = 0;
	for (std::cregex_iterator it{text, text + TEXT_SIZE - 32, r}, end; it != end; ++it)
		sum += static_cast<size_t>((*it)[2].length());
	sink = sum;
}

NI static auto substitute(void) -> void {
	std::regex r{"\\s+"};
	std::string out = std::regex_replace(std::string{text}, r, "_");
	sink = out.size();
}

NI static auto fields(void) -> void {
	std::regex r{","};
	size_t sum = 0;
	for (std::cregex_token_iterator it{csv, csv + TEXT_SIZE, r, -1}, end; it != end; ++it)
		++sum;
	sink = sum;
}

NI static auto nested(void) -> void {
	std::regex r{"(a|b|c)*d"};
	size_t sum = 0;
	for (int i = 0; i < 20; ++i)
		sum += std::regex_search(letters, letters + 4096, r) ? 1U : 0U;
	sink = sum;
}

#else
#include "5-collections/regex/regex.hpp"

NI static auto compile(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < 10000; ++i) {
		auto r = ca::regex::make(email);
		sum += r->group_count();
		static_cast<void>(r->cleanup());
	}
	sink = sum;
}

NI static auto literal(void) -> void {
	auto r = ca::regex::make("zebra crossings!");
	size_t sum = 0;
	for (int i = 0; i < 4; ++i) {
		auto m = r->search(ca::sv::str{text});
		sum += m && *m ? 1U : 0U;
	}
	sink = sum;
	static_cast<void>(r->cleanup());
}

template <typename F>
static auto each(ca::regex& r, const ca::sv::str subject, F f) -> void {
	auto matches = r.finditer(subject);
	while (true) {
		auto next = matches.next();
		if (!next || !*next)
			return;
		f(**next);
		static_cast<void>((*next)->cleanup());
	}
}

NI static auto words(void) -> void {
	auto r = ca::regex::make("\\w+");
	size_t sum = 0;
	each(*r, ca::sv::str{text, TEXT_SIZE - 32}, [&sum](const ca::regex_match<>& m) {
		sum += m.group().size();
	});
	sink = sum;
	static_cast<void>(r->cleanup());
}

NI static auto suffix(void) -> void {
	auto r = ca::regex::make("[a-z]+ing\\b");
	size_t sum = 0;
	each(*r, ca::sv::str{text, TEXT_SIZE - 32}, [&sum](const ca::regex_match<>&) {
		++sum;
	});
	sink = sum;
	static_cast<void>(r->cleanup());
}

NI static auto captures(void) -> void {
	auto r = ca::regex::make(email);
	size_t sum = 0;
	each(*r, ca::sv::str{text, TEXT_SIZE - 32}, [&sum](const ca::regex_match<>& m) {
		sum += m[2U].size();
	});
	sink = sum;
	static_cast<void>(r->cleanup());
}

NI static auto substitute(void) -> void {
	auto r = ca::regex::make("\\s+");
	auto out = r->sub("_", ca::sv::str{text});
	sink = out->size();
	static_cast<void>(out->cleanup());
	static_cast<void>(r->cleanup());
}

NI static auto fields(void) -> void {
	auto r = ca::regex::make(",");
	auto pieces = r->split(ca::sv::str{csv, TEXT_SIZE});
	sink = pieces->size();
	static_cast<void>(pieces->cleanup());
	static_cast<void>(r->cleanup());
}

NI static auto nested(void) -> void {
	auto r = ca::regex::make("(a|b|c)*d");
	size_t sum = 0;
	for (int i = 0; i < 20; ++i) {
		auto m = r->search(ca::sv::str{letters, 4096});
		sum += m && *m ? 1U : 0U;
	}
	sink = sum;
	static_cast<void>(r->cleanup());
}
#endif


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (!open_counters())
		return 1;
	make_text();
	section("compile");
	bench("compile ([a-z]+)@([a-z]+)\\.com, 1e4", "ca: parse, tree, NFA, analysis, scratch; std: its NFA", compile);
	section("search");
	bench("missing 16-char literal in 256 KiB, 4 times", "ca: no VM, one substring search", literal);
	bench("\\w+ over 256 KiB, every match", "slower: the VM steps every byte of each long match, std runs straight through", words);
	bench("[a-z]+ing\\b over 256 KiB, every match", "ca: one pass per match attempt, no backtracking", suffix);
	bench("([a-z]+)@([a-z]+)\\.com over 256 KiB, groups", "ca: captures in the VM threads", captures);
	bench("a repeated 3-way alternation then d, 4 KiB without d, 20 times", "std backtracks from every position: quadratic; ca: linear", nested);
	section("python");
	bench("sub(\\s+, _) of 256 KiB", substitute);
	bench("split(,) of 256 KiB", "ca: literal pattern, no VM", fields);
	return 0;
}
