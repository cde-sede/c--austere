// Benchmarks for ca::str and ca::unicode; -DUSE_STD runs the same cases on std::string,
// std::format and the C library's UTF-8 conversion (mbstowcs, locale C.UTF-8).

#include "measure.hpp"

#include <locale.h>
#include <stdlib.h>

static const int COUNT = 1000000;
static const int TEXT_SIZE = 1 << 20;
static char text[TEXT_SIZE + 1];
static wchar_t wide[TEXT_SIZE + 1];
static char csv[TEXT_SIZE + 1];
static const char* const numbers[] = {"12345", "42", "7x", "000123", "", "9876543210", "3.14", "65535"};

static auto make_text(void) -> void {
	static const char* const words[] = {"plain ", "ascii ", "caf\xC3\xA9 ", "\xE2\x82\xAC" "42 ", "text ", "\xF0\x9F\x98\x80 ", "lorem ", "ipsum "};
	int at = 0;
	for (int i = 0; at < TEXT_SIZE - 16; ++i) {
		const char* word = words[i % 3 == 0 ? (i / 3) % 8 : i % 2];
		while (*word != '\0')
			text[at++] = *word++;
	}
	text[at] = '\0';
	for (int i = 0; i < TEXT_SIZE; ++i)
		csv[i] = i % 9 == 8 ? ',' : static_cast<char>('a' + i % 23);
	csv[TEXT_SIZE] = '\0';
}

#ifdef USE_STD
#include <ctype.h>
#include <format>
#include <string_view>
#include <string>
#include <vector>

NI static auto construct_short(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < COUNT; ++i) {
		std::string s{"ten chars!"};
		sum += s.size() + static_cast<unsigned char>(s[i % 10]);
	}
	sink = sum;
}

NI static auto construct_medium(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < COUNT; ++i) {
		std::string s{"twenty-two characters!"};
		sum += s.size() + static_cast<unsigned char>(s[i % 22]);
	}
	sink = sum;
}

NI static auto push(void) -> void {
	std::string s;
	for (int i = 0; i < 10 * COUNT; ++i)
		s.push_back(static_cast<char>('a' + i % 26));
	sink = s.size();
}

NI static auto append(void) -> void {
	std::string s;
	for (int i = 0; i < COUNT; ++i)
		s.append("word, ", 6);
	sink = s.size();
}

NI static auto format(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < COUNT; ++i) {
		std::string s = std::format("{}={:04}", "key", i);
		sum += s.size();
	}
	sink = sum;
}

NI static auto array_of_strings(void) -> void {
	std::vector<std::string> list;
	for (int i = 0; i < COUNT; ++i)
		list.emplace_back(i % 2 == 0 ? "short" : "a string longer than inline");
	sink = list.size();
}

NI static auto find(void) -> void {
	const std::string haystack{text};
	size_t sum = 0;
	for (int i = 0; i < 20; ++i)
		sum += haystack.find("not there at all");
	sink = sum;
}

NI static auto count_points(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < 20; ++i)
		sum += mbstowcs(nullptr, text, 0);
	sink = sum;
}

NI static auto to_utf32(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < 20; ++i)
		sum += mbstowcs(wide, text, TEXT_SIZE);
	sink = sum + static_cast<size_t>(wide[7]);
}
NI static auto split_fields(void) -> void {
	const std::string_view line{csv};
	size_t sum = 0;
	for (int i = 0; i < 20; ++i) {
		size_t at = 0;
		for (;;) {
			const size_t comma = line.find(',', at);
			sum += (comma == std::string_view::npos ? line.size() : comma) - at;
			if (comma == std::string_view::npos)
				break;
			at = comma + 1;
		}
	}
	sink = sum;
}

NI static auto upper(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < 20; ++i) {
		std::string out;
		out.reserve(TEXT_SIZE);
		for (const char* at = csv; *at != '\0'; ++at)
			out.push_back(static_cast<char>(toupper(static_cast<unsigned char>(*at))));
		sum += out.size() + static_cast<unsigned char>(out[7]);
	}
	sink = sum;
}

NI static auto digits(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < COUNT; ++i) {
		const std::string_view field{numbers[i & 7]};
		bool all = !field.empty();
		for (const char c : field)
			all = all && isdigit(static_cast<unsigned char>(c)) != 0;
		sum += all ? 1U : 0U;
	}
	sink = sum;
}
#else
#include "2-vocabulary/unicode/count_code_points.hpp"
#include "2-vocabulary/unicode/transcode.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/format/format.hpp"
#include "5-collections/string/str.hpp"

NI static auto construct_short(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < COUNT; ++i) {
		auto s = ca::str::from("ten chars!");
		sum += s->size() + static_cast<unsigned char>((*s)[static_cast<ca::usz>(i % 10)]);
		static_cast<void>(s->destroy());
	}
	sink = sum;
}

NI static auto construct_medium(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < COUNT; ++i) {
		auto s = ca::str::from("twenty-two characters!");
		sum += s->size() + static_cast<unsigned char>((*s)[static_cast<ca::usz>(i % 22)]);
		static_cast<void>(s->destroy());
	}
	sink = sum;
}

NI static auto push(void) -> void {
	ca::str s;
	for (int i = 0; i < 10 * COUNT; ++i)
		static_cast<void>(s.push(static_cast<char>('a' + i % 26)));
	sink = s.size();
	static_cast<void>(s.destroy());
}

NI static auto append(void) -> void {
	ca::str s;
	for (int i = 0; i < COUNT; ++i)
		static_cast<void>(s.append(ca::sv::str{"word, ", 6U}));
	sink = s.size();
	static_cast<void>(s.destroy());
}

NI static auto format(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < COUNT; ++i) {
		auto s = ca::format("{}={:04}", "key", i);
		sum += s->size();
		static_cast<void>(s->destroy());
	}
	sink = sum;
}

NI static auto array_of_strings(void) -> void {
	ca::dynamic_array<ca::str> list;
	for (int i = 0; i < COUNT; ++i) {
		auto s = ca::str::from(i % 2 == 0 ? "short" : "a string longer than inline");
		static_cast<void>(list.append(static_cast<ca::str&&>(*s)));
	}
	sink = list.size();
	static_cast<void>(list.destroy());
}

NI static auto find(void) -> void {
	auto haystack = ca::str::from(text);
	size_t sum = 0;
	for (int i = 0; i < 20; ++i) {
		const auto at = haystack->view().find(ca::sv::str{"not there at all"});
		sum += at ? *at : ~size_t{0};
	}
	sink = sum;
	static_cast<void>(haystack->destroy());
}

NI static auto count_points(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < 20; ++i)
		sum += ca::unicode::count_code_points(ca::sv::str{text});
	sink = sum;
}

NI static auto to_utf32(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < 20; ++i)
		sum += ca::unicode::transcode(ca::sv::str{text}, wide, TEXT_SIZE).written();
	sink = sum + static_cast<size_t>(wide[7]);
}
NI static auto split_fields(void) -> void {
	const ca::sv::str line{csv};
	size_t sum = 0;
	for (int i = 0; i < 20; ++i) {
		for (const ca::sv::str field : line.split(","))
			sum += field.size();
	}
	sink = sum;
}

NI static auto upper(void) -> void {
	size_t sum = 0;
	auto text = ca::str::from(csv);
	for (int i = 0; i < 20; ++i) {
		auto out = text->upper();
		sum += out->size() + static_cast<unsigned char>((*out)[7]);
		static_cast<void>(out->destroy());
	}
	static_cast<void>(text->destroy());
	sink = sum;
}

NI static auto digits(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < COUNT; ++i)
		sum += ca::sv::str{numbers[i & 7]}.is_digit() ? 1U : 0U;
	sink = sum;
}
#endif


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (setlocale(LC_ALL, "C.UTF-8") == nullptr || !open_counters())
		return 1;
	make_text();
	section("str");
	bench("from() of 10 chars, 1e6", "inline in both; built inside the result: returning it copies nothing", construct_short);
	bench("from() of 22 chars, 1e6", "std::string allocates past 15", construct_medium);
	bench("push 1e7 chars", push);
	bench("append 1e6 6-char words", append);
	bench("ca::format vs std::format, short result, 1e6", format);
	bench("dynamic_array<str> vs vector<string>, 1e6 pushes, half inline", array_of_strings);
	bench("view().find of a missing 16-char needle in 1 MiB, 20 times", find);
	section("count_code_points");
	bench("count code points of 1 MiB of mixed UTF-8, 20 times", "std: mbstowcs(nullptr, ...)", count_points);
	section("transcode");
	bench("UTF-8 to UTF-32, 1 MiB of mixed text, 20 times", "std: mbstowcs", to_utf32);
	section("python");
	bench("split(\",\") of 1 MiB, 20 times", "std: a string_view::find loop", split_fields);
	bench("upper() of 1 MiB ASCII, 20 times", "std: push_back(toupper(c))", upper);
	bench("is_digit() of 1e6 short fields", "std: an isdigit loop; ca: one table read per ASCII code point", digits);
	return 0;
}
