// expect: exit 0
#include "2-vocabulary/fail.hpp"
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/array.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/map/hashmap.hpp"
#include "5-collections/map/is_map.hpp"

static_assert(ca::is_map<ca::hashmap<int, int>>);
static_assert(ca::hashmap<int, int>::TRIVIALLY_RELOCATABLE && sizeof(ca::hashmap<int, int>) == 5U * sizeof(void*));
static_assert(!ca::is_map<ca::dynamic_array<int>>);

static auto basics(void) noexcept -> int {

	ca::hashmap<int, int> m;
	if (!m.empty() || m.capacity() != 0U || m.get(1) || m.contains(1) || m.pop(1) || m.popitem() || m.begin() != m.end())
		return 1;
	if (!m.set(1, 10) || !m.set(2, 20) || !m.set(1, 11) || m.size() != 2U || m[1] != 11 || m[2] != 20)
		return 2;
	auto got = m.get(2);
	if (!got || *got != 20)
		return 3;
	*got = 21;
	if (m.get(2).value_or(0) != 21 || m.get(3).value_or(-1) != -1)
		return 4;
	auto defaulted = m.setdefault(3, 30);
	if (!defaulted || **defaulted != 30 || m.setdefault(3, 99).value_or(nullptr) != *defaulted || m[3] != 30)
		return 5;
	if (m.pop(2).value_or(0) != 21 || m.pop(2) || m.size() != 2U || m.contains(2))
		return 6;
	int keys = 0;
	int values = 0;
	for (const int key : m.keys())
		keys += key;
	for (int& value : m.values())
		values += value++;
	if (keys != 4 || values != 41 || m[1] != 12 || m[3] != 31)
		return 7;
	for (auto [key, value] : m.items())
		value = key * 100;
	if (m[1] != 100 || m[3] != 300)
		return 8;
	const auto& constant = m;
	if (constant[1] != 100 || constant.get(3).value_or(0) != 300)
		return 9;
	if (!m.retain([](const int key, int&) {
		return key != 1;
	}) || m.size() != 1U || m.contains(1))
		return 10;
	auto item = m.popitem();
	if (!item || item->key != 3 || item->value != 300 || !m.empty())
		return 11;
	return m.cleanup() && m.capacity() == 0U ? 0 : 12;
}

static auto many(void) noexcept -> int {

	constexpr int N = 20000;
	ca::hashmap<int, int> m;
	static bool present[N];
	static int expected[N];
	ca::u64 seed = 12345U;
	for (int round = 0; round < 200000; ++round) {
		seed = seed * 6364136223846793005U + 1442695040888963407U;
		const int key = static_cast<int>((seed >> 33U) % N);
		const int action = static_cast<int>((seed >> 20U) % 3U);
		if (action < 2) {
			if (!m.set(key, round))
				return 20;
			present[key] = true;
			expected[key] = round;
		}
		else {
			auto popped = m.pop(key);
			if (popped.has_value() != present[key] || (popped && *popped != expected[key]))
				return 21;
			present[key] = false;
		}
	}
	ca::usz count = 0U;
	for (int key = 0; key < N; ++key) {
		if (m.contains(key) != present[key] || (present[key] && m[key] != expected[key]))
			return 22;
		count += present[key] ? 1U : 0U;
	}
	if (m.size() != count)
		return 23;
	ca::usz walked = 0U;
	for (const auto [key, value] : m) {
		if (!present[key] || expected[key] != value)
			return 24;
		++walked;
	}
	if (walked != count)
		return 25;
	auto copy = m.clone();
	if (!copy || !(*copy == m) || copy->capacity() != m.capacity())
		return 26;
	if (!copy->set(-1, 0) || *copy == m)
		return 27;
	const ca::usz before = m.capacity();
	if (!m.shrink() || m.capacity() > before || m.size() != count)
		return 28;
	for (int key = 0; key < N; ++key) {
		if (m.contains(key) != present[key])
			return 29;
	}
	if (!m.reserve(100000U) || m.capacity() < 100000U || m.size() != count)
		return 30;
	if (!m.update(*copy) || m.size() != count + 1U || m[-1] != 0)
		return 31;
	ca::hashmap<int, int> moved;
	if (!moved.set(-2, 5) || !moved.set(-1, 7) || !m.update(ca::move(moved)) || !moved.empty() || m[-1] != 7 || m[-2] != 5)
		return 32;
	if (!m.clear() || !m.empty() || m.capacity() == 0U || m.contains(0))
		return 33;
	return m.cleanup() && copy->cleanup() ? 0 : 34;
}

static auto from_keys(void) noexcept -> int {

	auto made = ca::hashmap<int, int>::from_keys(ca::array{3, 1, 3, 2}, 7);
	if (!made || made->size() != 3U || (*made)[1] != 7 || (*made)[2] != 7 || (*made)[3] != 7)
		return 40;
	return made->cleanup() ? 0 : 41;
}

static int cleanups = 0;
static int clones = 0;

struct tracked {
	int code;
	auto cleanup(void) noexcept -> ca::result<void> {
		++cleanups;
		if (code != 0)
			return ca::fail(code);
		return ca::result<void>{};
	}
	auto clone(void) const noexcept -> ca::result<tracked> {
		if (++clones == 3)
			return ca::fail(9);
		return tracked{code};
	}
};

static auto cleanup(void) noexcept -> int {

	ca::hashmap<int, tracked> m;
	for (int i = 0; i < 4; ++i)
		if (!m.set(i, tracked{0}))
			return 50;
	auto copy = m.clone();
	if (copy || copy.error().code() != 9 || cleanups != 2)
		return 51;
	cleanups = 0;
	auto replaced = m.set(0, tracked{0});
	if (!replaced || cleanups != 1)
		return 52;
	if (!m.set(1, tracked{5}))
		return 53;
	auto failed = m.clear();
	if (failed || failed.error().code() != 5 || !m.empty())
		return 54;
	ca::hashmap<int, ca::dynamic_array<int>> nested;
	ca::dynamic_array<int> inner;
	if (!inner.append(1) || !nested.set(4, ca::move(inner)) || nested[4][0] != 1)
		return 55;
	auto cloned = nested.clone();
	if (!cloned || (*cloned)[4][0] != 1)
		return 56;
	return m.cleanup() && nested.cleanup() && cloned->cleanup() ? 0 : 57;
}

static auto arena(void) noexcept -> int {

	auto made = ca::arena::make();
	if (!made)
		return 60;
	ca::arena arena = ca::move(*made);
	{
		auto m = ca::make_in<ca::hashmap<int, int>>(arena);
		if (!m)
			return 61;
		for (int i = 0; i < 1000; ++i)
			if (!m->set(i, -i))
				return 62;
		if (m->size() != 1000U || (*m)[999] != -999 || !m->cleanup())
			return 63;
	}
	return arena.cleanup() ? 0 : 64;
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
