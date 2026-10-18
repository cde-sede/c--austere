// expect: exit 0
#include "2-vocabulary/fail.hpp"
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/array.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/list/list.hpp"

static_assert(ca::list<int>::TRIVIALLY_RELOCATABLE && sizeof(ca::list<int>) == 3U * sizeof(void*));

template <typename L, ca::usz N>
static auto holds(const L& list, const ca::array<int, N>& expected) noexcept -> bool {

	if (list.size() != N)
		return false;
	ca::usz i = 0U;
	for (const int value : list) {
		if (value != expected[i++])
			return false;
	}
	i = N;
	for (auto at = list.end(); at != list.begin();) {
		--at;
		if (*at != expected[--i])
			return false;
	}
	return true;
}

static auto basics(void) noexcept -> int {

	ca::list<int> l;
	if (l.first() || l.pop() || l.pop_front() || l.begin() != l.end())
		return 1;
	if (!l.extend({5, 3, 9}) || !l.prepend(1) || !l.append(7) || !holds(l, ca::array{1, 5, 3, 9, 7}))
		return 2;
	const int* const pinned = &*l.find(3).value_or(l.begin());
	l.sort();
	if (!holds(l, ca::array{1, 3, 5, 7, 9}) || &*l.find(3).value_or(l.begin()) != pinned)
		return 3;
	l.reverse();
	if (!holds(l, ca::array{9, 7, 5, 3, 1}) || l.first().value_or(0) != 9 || l.last().value_or(0) != 1)
		return 4;
	auto at = l.find(5);
	if (!at)
		return 5;
	auto added = l.insert(*at, 42);
	if (!added || **added != 42 || !l.insert(l.end(), 0) || !holds(l, ca::array{9, 7, 42, 5, 3, 1, 0}))
		return 6;
	if (l.pop(*added).value_or(0) != 42 || l.pop(l.end()) || l.remove(3).value_or(0) != 3 || l.remove(3))
		return 7;
	if (!l.retain([](const int& value) {
		return value % 2 == 1;
	}) || !holds(l, ca::array{9, 7, 5, 1}))
		return 8;
	ca::list<int> other;
	if (!other.extend({100, 200}))
		return 9;
	l.splice(l.find(5).value_or(l.end()), ca::move(other));
	if (!other.empty() || !holds(l, ca::array{9, 7, 100, 200, 5, 1}) || l.count(100) != 1U || !l.contains(200))
		return 10;
	auto copy = l.clone();
	if (!copy || !holds(*copy, ca::array{9, 7, 100, 200, 5, 1}))
		return 11;
	l.sort([](const int& left, const int& right) {
		return left > right;
	});
	if (!holds(l, ca::array{200, 100, 9, 7, 5, 1}) || l.pop().value_or(0) != 1 || l.pop_front().value_or(0) != 200)
		return 12;
	return l.destroy() && l.empty() && copy->destroy() ? 0 : 13;
}

struct keyed {
	int key;
	int order;
};

static auto stable_sort(void) noexcept -> int {

	ca::list<keyed> l;
	for (int i = 0; i < 100; ++i)
		if (!l.append(keyed{(i * 37) % 5, i}))
			return 20;
	l.sort([](const keyed& left, const keyed& right) {
		return left.key < right.key;
	});
	keyed previous{-1, -1};
	for (const keyed& k : l) {
		if (k.key < previous.key || (k.key == previous.key && k.order < previous.order))
			return 21;
		previous = k;
	}
	return l.size() == 100U && l.destroy() ? 0 : 22;
}

static int cleanups = 0;

struct cleanable {
	int code;
	auto destroy(void) noexcept -> ca::result<void> {
		++cleanups;
		if (code != 0)
			return ca::fail(code);
		return ca::result<void>{};
	}
};

static auto cleanup(void) noexcept -> int {

	ca::list<cleanable> l;
	if (!l.append(cleanable{0}) || !l.append(cleanable{3}) || !l.append(cleanable{4}))
		return 30;
	auto retained = l.retain([](const cleanable& c) {
		return c.code == 0;
	});
	if (retained || retained.error().code() != 3 || cleanups != 2 || l.size() != 1U)
		return 31;
	ca::list<ca::dynamic_array<int>> nested;
	ca::dynamic_array<int> inner;
	if (!inner.append(1) || !nested.append(ca::move(inner)))
		return 32;
	auto copy = nested.clone();
	if (!copy)
		return 33;
	auto first = copy->first();
	if (!first || (*first)[0] != 1)
		return 33;
	return l.destroy() && cleanups == 3 && nested.destroy() && copy->destroy() ? 0 : 34;
}

static auto arena(void) noexcept -> int {

	auto made = ca::arena::make();
	if (!made)
		return 40;
	ca::arena arena = ca::move(*made);
	{
		auto l = ca::make_in<ca::list<int>>(arena);
		if (!l)
			return 41;
		for (int i = 0; i < 1000; ++i)
			if (!l->prepend(i))
				return 42;
		l->sort();
		if (l->first().value_or(-1) != 0 || l->last().value_or(-1) != 999 || !l->destroy())
			return 43;
	}
	return arena.destroy() ? 0 : 44;
}

auto main(void) -> int {

	if (const int failed = basics(); failed != 0)
		return failed;
	if (const int failed = stable_sort(); failed != 0)
		return failed;
	if (const int failed = cleanup(); failed != 0)
		return failed;
	return arena();
}
