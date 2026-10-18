// expect: exit 0
#include "1-primitives/traits/is_trivially_copyable.hpp"
#include "3-system/error/errc.hpp"
#include "5-collections/string/inline_str.hpp"

static_assert(sizeof(ca::inline_str<15>) == 17U);
static_assert(sizeof(ca::inline_str<300>) == 304U);
static_assert(ca::is_trivially_copyable<ca::inline_str<8, char16_t>>);

constexpr auto build(void) noexcept -> bool {

	auto s = ca::inline_str<13>::from("hello");
	if (!s || !s->append(", you") || s->size() != 10U || *s != ca::sv::str{"hello, you"})
		return false;
	if (s->append("!!!!") || *s != ca::sv::str{"hello, you"})
		return false;
	if (!s->insert(5U, s->view().prefix(2U)) || *s != ca::sv::str{"hellohe, you"} || s->size() != 12U)
		return false;
	s->remove(5U, 2U);
	if (!s->insert(0U, s->view().slice(7U, 3U)) || *s != ca::sv::str{"youhello, you"} || !s->full())
		return false;
	return true;
}

template <typename T>
constexpr auto fails_with(const ca::result<T>& result, const ca::error error) noexcept -> bool {
	return !result && result.error() == error;
}

static_assert(build());

constexpr auto insert_overflow(void) noexcept -> bool {

	ca::inline_str<4> s;
	return s.assign("abcd") && !s.push('e') && fails_with(s.insert(1U, "x"), ca::errc::no_space) && s == ca::sv::str{"abcd"};
}

static_assert(insert_overflow());

constexpr auto wide(void) noexcept -> bool {

	ca::inline_str<3, char16_t> w;
	return w.append_code_point(U'\U0001F600') && w.size() == 2U && !w.append_code_point(U'\U0001F600') && w.append_code_point(U'x') && w.size() == 3U && w.full();
}

static_assert(wide());

auto main(void) -> int {

	if (!build())
		return 1;
	ca::inline_str<8> s;
	if (!s.resize(3U, 'z') || s != ca::sv::str{"zzz"} || s.c_str()[3] != '\0')
		return 2;
	ca::inline_str<8> copy = s;
	copy[0] = 'a';
	if (s[0] != 'z' || copy != ca::sv::str{"azz"})
		return 3;
	auto last = copy.pop();
	if (!last || *last != 'z' || copy.size() != 2U || copy.get(2U))
		return 4;
	if (s.resize(9U))
		return 5;
	copy.clear();
	return copy.empty() && fails_with(ca::inline_str<3>::from("four"), ca::errc::no_space) ? 0 : 6;
}
