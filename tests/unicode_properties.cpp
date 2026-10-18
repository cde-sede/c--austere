// expect: exit 0
#include "2-vocabulary/unicode/properties_of.hpp"

constexpr auto maps_to(const ca::unicode::mapping mapping, const char32_t a, const char32_t b = U'\0', const char32_t c = U'\0') noexcept -> bool {

	const char32_t expected[3] = {a, b, c};
	const ca::usz size = b == U'\0' ? 1U : c == U'\0' ? 2U : 3U;
	if (mapping.size() != size)
		return false;
	for (ca::usz i = 0U; i < size; ++i)
		if (mapping.begin()[i] != expected[i])
			return false;
	return true;
}

static_assert(ca::unicode::properties_of(U'é').is_alpha());
static_assert(ca::unicode::properties_of(U'é').is_lower());
static_assert(ca::unicode::properties_of(U'é').to_upper() == U'É');
static_assert(ca::unicode::properties_of(U'É').to_lower() == U'é');
static_assert(ca::unicode::properties_of(U'٣').is_decimal());
static_assert(ca::unicode::properties_of(U'²').is_digit() && !ca::unicode::properties_of(U'²').is_decimal());
static_assert(ca::unicode::properties_of(U'½').is_numeric() && !ca::unicode::properties_of(U'½').is_digit());
static_assert(ca::unicode::properties_of(U'一').is_numeric());
static_assert(ca::unicode::properties_of(U' ').is_space() && !ca::unicode::properties_of(U' ').is_printable());
static_assert(ca::unicode::properties_of(U' ').is_line_break());
static_assert(ca::unicode::properties_of(U'\u0085').is_line_break());
static_assert(ca::unicode::properties_of(U'ǅ').is_title() && ca::unicode::properties_of(U'ǅ').is_cased());
static_assert(ca::unicode::properties_of(U'­').is_case_ignorable());
static_assert(ca::unicode::properties_of(U'́').is_case_ignorable() && ca::unicode::properties_of(U'́').is_identifier_continue());
static_assert(!ca::unicode::properties_of(U'́').is_identifier_start());
static_assert(ca::unicode::properties_of(U'\U0001F600').is_printable() && !ca::unicode::properties_of(U'\U0001F600').is_alpha());
static_assert(ca::unicode::properties_of(U'\U0001D400').is_upper());
static_assert(ca::unicode::properties_of(U'\U000E0100').is_case_ignorable());
static_assert(!ca::unicode::properties_of(U'\U000F0000').is_printable());
static_assert(ca::unicode::properties_of(static_cast<char32_t>(0x110000U)).to_upper() == static_cast<char32_t>(0x110000U));

static_assert(maps_to(ca::unicode::properties_of(U'ß').upper(), U'S', U'S'));
static_assert(maps_to(ca::unicode::properties_of(U'ß').title(), U'S', U's'));
static_assert(maps_to(ca::unicode::properties_of(U'ß').fold(), U's', U's'));
static_assert(ca::unicode::properties_of(U'ß').to_upper() == U'ß');
static_assert(maps_to(ca::unicode::properties_of(U'İ').lower(), U'i', U'̇'));
static_assert(ca::unicode::properties_of(U'İ').to_lower() == U'i');
static_assert(maps_to(ca::unicode::properties_of(U'ΐ').upper(), U'Ι', U'̈', U'́'));
static_assert(maps_to(ca::unicode::properties_of(U'ẞ').fold(), U's', U's'));
static_assert(ca::unicode::properties_of(U'ẞ').to_fold() == U'ß');
static_assert(ca::unicode::properties_of(U'ǆ').to_title() == U'ǅ');
static_assert(ca::unicode::properties_of(U'Σ').to_lower() == U'σ');
static_assert(ca::unicode::properties_of(U'\U00010400').to_lower() == U'\U00010428');

auto main(void) -> int {
	return 0;
}
