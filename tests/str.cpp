// expect: exit 0
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/traits/is_default_constructible.hpp"
#include "1-primitives/utility/move.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/is_clonable.hpp"
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/string/str.hpp"
#include "5-collections/string/u16str.hpp"
#include "5-collections/string/u32str.hpp"
#include "5-collections/string/u8str.hpp"
#include "5-collections/string/wstr.hpp"

static_assert(sizeof(ca::str) == 3U * sizeof(void*));
static_assert(ca::str::INLINE_CAPACITY == 23U && ca::u16str::INLINE_CAPACITY == 11U && ca::u32str::INLINE_CAPACITY == 5U);
static_assert(ca::is_trivially_relocatable<ca::str> && ca::is_cleanable<ca::str> && ca::is_clonable<ca::str>);
static_assert(!ca::is_default_constructible<ca::basic_str<char, ca::arena_allocator>>);

static auto is(const ca::str& s, const ca::sv::str text) noexcept -> bool {
	return s == text && s.c_str()[s.size()] == '\0';
}

static auto growth(void) noexcept -> int {

	ca::str s;
	for (int i = 0; i < 23; ++i)
		if (!s.push(static_cast<char>('a' + i % 26)))
			return 1;
	if (s.capacity() != 23U || !is(s, "abcdefghijklmnopqrstuvw"))
		return 2;
	if (!s.push('x') || s.capacity() < 24U || !is(s, "abcdefghijklmnopqrstuvwx"))
		return 3;
	if (!s.append(s.view()) || !is(s, "abcdefghijklmnopqrstuvwxabcdefghijklmnopqrstuvwx"))
		return 4;
	s.remove(3U, 40U);
	if (!is(s, "abctuvwx"))
		return 5;
	if (!s.shrink() || s.capacity() != 23U || !is(s, "abctuvwx"))
		return 6;
	if (!s.insert(0U, s.view().slice(3U, 3U)) || !is(s, "tuvabctuvwx"))
		return 7;
	if (!s.insert(11U, "!") || !s.insert(3U, "-") || !is(s, "tuv-abctuvwx!"))
		return 8;
	if (!s.assign(s.view().without_prefix(4U)) || !is(s, "abctuvwx!"))
		return 9;
	auto last = s.pop();
	if (!last || *last != '!' || !s.resize(12U, '.') || !is(s, "abctuvwx...."))
		return 10;
	if (!s.resize(2U) || !is(s, "ab") || s[1] != 'b' || s.get(2U))
		return 11;
	s.clear();
	if (!s.empty() || !is(s, ""))
		return 12;
	return s.destroy() ? 0 : 13;
}

static auto ownership(void) noexcept -> int {

	auto made = ca::str::from("a string long enough to live on the heap");
	if (!made)
		return 20;
	ca::str moved = ca::move(*made);
	if (!made->empty() || !is(moved, "a string long enough to live on the heap"))
		return 21;
	auto copy = moved.clone();
	if (!copy || *copy != moved || copy->data() == moved.data())
		return 22;
	ca::str small{};
	if (!small.assign("tiny"))
		return 23;
	moved = ca::move(small);
	if (!is(moved, "tiny") || !small.empty())
		return 24;
	const ca::sv::str view = *copy;
	if (!view.starts_with("a string") || view.find(ca::sv::str{"heap"}) != 36U)
		return 25;
	ca::dynamic_array<ca::str> list;
	for (int i = 0; i < 100; ++i) {
		auto item = ca::str::from(i % 2 == 0 ? ca::sv::str{"short"} : ca::sv::str{"a longer string, allocated"});
		if (!item || !list.append(ca::move(*item)))
			return 26;
	}
	if (list[99] != ca::sv::str{"a longer string, allocated"} || list[98] != ca::sv::str{"short"})
		return 27;
	return list.destroy() && copy->destroy() && moved.destroy() ? 0 : 28;
}

static auto encodings(void) noexcept -> int {

	ca::u16str w;
	if (!w.append(u"caf") || !w.append_code_point(U'\u00e9') || !w.append_code_point(U'\U0001F600'))
		return 30;
	if (w.size() != 6U || w != ca::sv::u16{u"caf\u00e9\U0001F600"})
		return 31;
	auto u = ca::u32str::from(U"0123456789");
	if (!u || u->size() != 10U || (*u)[9] != U'9' || u->c_str()[10] != U'\0')
		return 32;
	auto e = ca::u8str::from(u8"\u00e9");
	auto l = ca::wstr::from(L"wide");
	if (!e || e->size() != 2U || !l || l->view().find(L'd') != 2U)
		return 33;
	return w.destroy() && u->destroy() && e->destroy() && l->destroy() ? 0 : 34;
}

static auto arena(void) noexcept -> int {

	auto made = ca::arena::make();
	if (!made)
		return 40;
	ca::arena arena = ca::move(*made);
	auto s = ca::make_in<ca::str>(arena, ca::sv::str{"in the arena, past the inline capacity"});
	if (!s || s->size() != 38U)
		return 41;
	if (!s->append(" and more") || *s != ca::sv::str{"in the arena, past the inline capacity and more"})
		return 42;
	return s->destroy() && arena.destroy() ? 0 : 43;
}

auto main(void) -> int {

	if (const int failed = growth(); failed != 0)
		return failed;
	if (const int failed = ownership(); failed != 0)
		return failed;
	if (const int failed = encodings(); failed != 0)
		return failed;
	return arena();
}
