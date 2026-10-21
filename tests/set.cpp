// expect: exit 0
#include "5-collections/array/array.hpp"
#include "5-collections/map/set.hpp"
#include "5-collections/string/str.hpp"

static_assert(ca::set<int>::TRIVIALLY_RELOCATABLE && sizeof(ca::set<int>) == 5U * sizeof(void*));

template <ca::usz N>
static auto holds(const ca::set<int>& s, const ca::array<int, N>& expected) noexcept -> bool {

	if (s.size() != N)
		return false;
	for (const int key : expected) {
		if (!s.contains(key))
			return false;
	}
	return true;
}

static auto basics(void) noexcept -> int {

	ca::set<int> s;
	if (!s.empty() || s.pop() || s.remove(1) || s.discard(1) || s.begin() != s.end())
		return 1;
	if (!s.add(1) || !s.add(2) || !s.add(1) || !holds(s, ca::array{1, 2}))
		return 2;
	if (!s.discard(1) || s.discard(1) || s.remove(2).value_or(0) != 2 || !s.empty())
		return 3;
	auto a = ca::set<int>::make(ca::array{1, 2, 3, 4, 2});
	auto b = ca::set<int>::make(ca::array{3, 4, 5});
	if (!a || !b || !holds(*a, ca::array{1, 2, 3, 4}))
		return 4;
	auto both = a->union_with(*b);
	auto common = a->intersection(*b);
	auto only = a->difference(*b);
	auto either = a->symmetric_difference(*b);
	if (!both || !common || !only || !either)
		return 5;
	if (!holds(*both, ca::array{1, 2, 3, 4, 5}) || !holds(*common, ca::array{3, 4}) || !holds(*only, ca::array{1, 2})
	 || !holds(*either, ca::array{1, 2, 5}))
		return 6;
	if (!common->is_subset(*a) || !a->is_superset(*common) || a->is_subset(*b) || !only->is_disjoint(*b) || a->is_disjoint(*b))
		return 7;
	auto c = a->clone();
	if (!c || !(*c == *a) || *c == *b)
		return 8;
	if (!c->intersection_update(*b) || !holds(*c, ca::array{3, 4}))
		return 9;
	if (!c->symmetric_difference_update(*b) || !holds(*c, ca::array{5}))
		return 10;
	if (!c->update(*a) || !c->difference_update(*b) || !holds(*c, ca::array{1, 2}))
		return 11;
	ca::set<int> moved;
	if (!moved.add(2) || !moved.add(9) || !c->update(ca::move(moved)) || !moved.empty() || !holds(*c, ca::array{1, 2, 9}))
		return 12;
	int sum = 0;
	for (const int key : *c)
		sum += key;
	if (sum != 12 || c->get(9).value_or(0) != 9)
		return 13;
	while (c->pop()) {
	}
	if (!c->empty())
		return 14;
	return a->cleanup() && b->cleanup() && both->cleanup() && common->cleanup() && only->cleanup() && either->cleanup() && c->cleanup() ? 0 : 15;
}

static auto many(void) noexcept -> int {

	ca::set<ca::u64> s;
	for (ca::u64 i = 0U; i < 100000U; ++i)
		if (!s.add(i * 7919U))
			return 20;
	for (ca::u64 i = 0U; i < 100000U; i += 3U)
		if (!s.discard(i * 7919U))
			return 21;
	for (ca::u64 i = 0U; i < 100000U; ++i)
		if (s.contains(i * 7919U) != (i % 3U != 0U) || s.contains(i * 7919U + 1U))
			return 22;
	return s.size() == 66666U && s.cleanup() ? 0 : 23;
}

static auto text(void) noexcept -> int {

	ca::set<ca::str> s;
	if (!s.add("apple") || !s.add("pear") || !s.add(ca::sv::str{"apple"}) || s.size() != 2U || !s.contains("pear"))
		return 30;
	auto taken = s.remove("apple");
	if (!taken || *taken != "apple" || !taken->cleanup() || s.contains("apple"))
		return 31;
	return s.cleanup() ? 0 : 32;
}

auto main(void) -> int {

	if (const int failed = basics(); failed != 0)
		return failed;
	if (const int failed = many(); failed != 0)
		return failed;
	return text();
}
