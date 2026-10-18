// expect: exit 0
#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/fail.hpp"
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/array.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/map/is_map.hpp"
#include "5-collections/map/map.hpp"

static_assert(ca::is_map<ca::map<int, int>>);
static_assert(ca::map<int, int>::TRIVIALLY_RELOCATABLE && sizeof(ca::map<int, int>) == 2U * sizeof(void*));

template <typename M, ca::usz N>
static auto holds(const M& m, const ca::array<int, N>& keys) noexcept -> bool {

	if (m.size() != N)
		return false;
	ca::usz i = 0U;
	for (const int key : m.keys()) {
		if (key != keys[i++])
			return false;
	}
	return i == N;
}

static auto basics(void) noexcept -> int {

	ca::map<int, int> m;
	if (!m.empty() || m.get(1) || m.first() || m.last() || m.pop(1) || m.popitem() || m.pop_first() || m.begin() != m.end())
		return 1;
	if (!m.set(5, 50) || !m.set(1, 10) || !m.set(3, 30) || !m.set(1, 11) || !holds(m, ca::array{1, 3, 5}) || m[1] != 11)
		return 2;
	auto first = m.first();
	auto last = m.last();
	if (!first || first->key != 1 || first->value != 11 || !last || last->key != 5)
		return 3;
	if (m.floor(4).value_or(*first).key != 3 || m.floor(3).value_or(*first).key != 3 || m.floor(0)
	 || m.ceiling(4).value_or(*first).key != 5 || m.ceiling(6) || m.ceiling(-9).value_or(*last).key != 1)
		return 4;
	auto defaulted = m.setdefault(4, 40);
	if (!defaulted || **defaulted != 40 || m.setdefault(4, 0).value_or(nullptr) != *defaulted || !holds(m, ca::array{1, 3, 4, 5}))
		return 5;
	for (auto [key, value] : m)
		value = key * 2;
	int values = 0;
	for (const int value : m.values())
		values += value;
	if (values != 26)
		return 6;
	if (m.pop(3).value_or(0) != 6 || m.pop(3) || !holds(m, ca::array{1, 4, 5}))
		return 7;
	auto top = m.popitem();
	auto bottom = m.pop_first();
	if (!top || top->key != 5 || !bottom || bottom->key != 1 || !holds(m, ca::array{4}))
		return 8;
	const auto& constant = m;
	if (constant[4] != 8 || constant.get(4).value_or(0) != 8 || !constant.contains(4))
		return 9;
	return m.destroy() && m.empty() ? 0 : 10;
}

static auto many(void) noexcept -> int {

	constexpr int N = 5000;
	static bool present[N];
	static int expected[N];
	ca::map<int, int> m;
	ca::u64 seed = 777U;
	for (int round = 0; round < 300000; ++round) {
		seed = seed * 6364136223846793005U + 1442695040888963407U;
		const int key = static_cast<int>((seed >> 33U) % N);
		const ca::u64 action = (seed >> 20U) % 5U;
		if (action < 3U) {
			if (!m.set(key, round))
				return 20;
			present[key] = true;
			expected[key] = round;
		}
		else if (action == 3U) {
			auto popped = m.pop(key);
			if (popped.has_value() != present[key] || (popped && *popped != expected[key]))
				return 21;
			present[key] = false;
		}
		else {
			int floor = key;
			while (floor >= 0 && !present[floor])
				--floor;
			auto found = m.floor(key);
			if (found.has_value() != (floor >= 0) || (found && found->key != floor))
				return 22;
			int ceiling = key;
			while (ceiling < N && !present[ceiling])
				++ceiling;
			auto above = m.ceiling(key);
			if (above.has_value() != (ceiling < N) || (above && above->key != ceiling))
				return 23;
		}
	}
	ca::usz count = 0U;
	for (int key = 0; key < N; ++key) {
		if (m.contains(key) != present[key] || (present[key] && m[key] != expected[key]))
			return 24;
		count += present[key] ? 1U : 0U;
	}
	int previous = -1;
	ca::usz walked = 0U;
	for (const auto [key, value] : m) {
		if (key <= previous || !present[key] || expected[key] != value)
			return 25;
		previous = key;
		++walked;
	}
	if (m.size() != count || walked != count)
		return 26;
	auto copy = m.clone();
	if (!copy || !(*copy == m))
		return 27;
	if (!m.retain([](const int key, int&) {
		return key % 3 != 0;
	}))
		return 28;
	for (int key = 0; key < N; ++key) {
		if (m.contains(key) != (present[key] && key % 3 != 0))
			return 29;
	}
	if (!m.update(*copy) || !(*copy == m))
		return 30;
	while (auto popped = m.pop_first()) {
	}
	if (!m.empty() || m.begin() != m.end())
		return 31;
	ca::map<int, int> moved;
	if (!moved.set(1, 1) || !moved.set(2, 2) || !copy->update(ca::move(moved)) || !moved.empty() || (*copy)[2] != 2)
		return 32;
	return m.destroy() && copy->destroy() ? 0 : 33;
}

static auto from_keys(void) noexcept -> int {

	auto made = ca::map<int, int>::from_keys(ca::array{3, 1, 3, 2}, 7);
	if (!made || !holds(*made, ca::array{1, 2, 3}) || (*made)[2] != 7)
		return 40;
	return made->destroy() ? 0 : 41;
}

static int cleanups = 0;
static int clones = 0;

struct tracked {
	int code;
	auto destroy(void) noexcept -> ca::result<void> {
		++cleanups;
		if (code != 0)
			return ca::fail(code);
		return ca::result<void>{};
	}
	auto clone(void) const noexcept -> ca::result<tracked> {
		if (++clones == 30)
			return ca::fail(9);
		return tracked{code};
	}
};

static auto cleanup(void) noexcept -> int {

	ca::map<int, tracked> m;
	for (int i = 0; i < 100; ++i)
		if (!m.set(i, tracked{0}))
			return 50;
	auto copy = m.clone();
	if (copy || copy.error().code() != 9 || cleanups != 29)
		return 51;
	cleanups = 0;
	if (!m.set(0, tracked{0}) || cleanups != 1 || !m.set(1, tracked{5}))
		return 52;
	cleanups = 0;
	auto kept = m.retain([](const int key, tracked&) {
		return key >= 50;
	});
	if (kept || kept.error().code() != 5 || cleanups != 50 || m.size() != 50U)
		return 53;
	ca::map<int, ca::dynamic_array<int>> nested;
	ca::dynamic_array<int> inner;
	if (!inner.append(1) || !nested.set(4, ca::move(inner)))
		return 54;
	auto cloned = nested.clone();
	if (!cloned || (*cloned)[4][0] != 1)
		return 55;
	return m.destroy() && nested.destroy() && cloned->destroy() ? 0 : 56;
}

static auto arena(void) noexcept -> int {

	auto made = ca::arena::make();
	if (!made)
		return 60;
	ca::arena arena = ca::move(*made);
	{
		auto m = ca::make_in<ca::map<int, int>>(arena);
		if (!m)
			return 61;
		for (int i = 1000; i > 0; --i)
			if (!m->set(i, -i))
				return 62;
		auto first = m->first();
		if (m->size() != 1000U || !first || first->key != 1 || !m->destroy())
			return 63;
	}
	return arena.destroy() ? 0 : 64;
}

auto main(void) -> int {

	if (const int failed = basics(); failed != 0)
		return failed;
	if (const int failed = many(); failed != 0)
		return failed;
	if (const int failed = from_keys(); failed != 0)
		return failed;
	if (const int failed = cleanup(); failed != 0)
		return failed;
	return arena();
}
