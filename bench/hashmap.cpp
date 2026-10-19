// Benchmarks for ca::hashmap; -DUSE_STD runs the same cases on std::unordered_map.

#include "measure.hpp"

#ifdef USE_STD
#include <string>
#include <string_view>
#include <unordered_map>

template <typename K, typename V>
struct table {
	std::unordered_map<K, V> m;
	auto set(K k, V v) -> bool { m[k] = v; return true; }
	auto get(const K& k) const -> const V* { auto it = m.find(k); return it == m.end() ? nullptr : &it->second; }
	auto pop(const K& k) -> bool { return m.erase(k) != 0; }
	auto size(void) const -> size_t { return m.size(); }
	template <typename F> auto each(F f) const -> void { for (const auto& [k, v] : m) f(k, v); }
	auto cleanup(void) -> bool { m.clear(); return true; }
};

struct text_table {
	std::unordered_map<std::string, int> m;
	auto set(const char* k, int v) -> bool { m[k] = v; return true; }
	auto get(const char* k) const -> const int* { auto it = m.find(k); return it == m.end() ? nullptr : &it->second; }
	auto cleanup(void) -> bool { m.clear(); return true; }
};
#else
#include "5-collections/map/hashmap.hpp"
#include "5-collections/string/str.hpp"

template <typename K, typename V>
struct table {
	ca::hashmap<K, V> m;
	auto set(K k, V v) -> bool { return static_cast<bool>(m.set(k, v)); }
	auto get(const K& k) const -> const V* { auto r = m.get(k); return r ? &*r : nullptr; }
	auto pop(const K& k) -> bool { return m.pop(k).has_value(); }
	auto size(void) const -> size_t { return m.size(); }
	template <typename F> auto each(F f) const -> void { for (const auto [k, v] : m) f(k, v); }
	auto cleanup(void) -> bool { return static_cast<bool>(m.cleanup()); }
};

struct text_table {
	ca::hashmap<ca::str, int> m;
	auto set(const char* k, int v) -> bool { return static_cast<bool>(m.set(k, v)); }
	auto get(const char* k) const -> const int* { auto r = m.get(k); return r ? &*r : nullptr; }
	auto cleanup(void) -> bool { return static_cast<bool>(m.cleanup()); }
};
#endif

#define CHECK(x) do { if (!(x)) __builtin_trap(); } while (0)


// -- CASES ------------------------------------------------------------------

static const int N = 1000000;

static auto scrambled(const uint64_t i) -> uint64_t {
	return (i * 0x9E3779B97F4A7C15U) ^ (i >> 7U);
}

NI static auto insert_cleanup(void) -> void {
	table<uint64_t, uint64_t> t;
	for (int i = 0; i < N; ++i)
		CHECK(t.set(scrambled(static_cast<uint64_t>(i)), static_cast<uint64_t>(i)));
	sink = t.size();
	CHECK(t.cleanup());
}

NI static auto lookup_hits(const table<uint64_t, uint64_t>& t) -> void {
	uint64_t sum = 0;
	for (int r = 0; r < 10; ++r)
		for (int i = 0; i < N; ++i)
			sum += *t.get(scrambled(static_cast<uint64_t>(i) * 7919U % N));
	sink = sum;
}

NI static auto lookup_misses(const table<uint64_t, uint64_t>& t) -> void {
	uint64_t found = 0;
	for (int r = 0; r < 10; ++r)
		for (int i = 0; i < N; ++i)
			found += t.get(scrambled(static_cast<uint64_t>(N + i))) != nullptr ? 1U : 0U;
	sink = found;
}

NI static auto churn(void) -> void {
	table<uint64_t, uint64_t> t;
	for (int i = 0; i < N / 10; ++i)
		CHECK(t.set(scrambled(static_cast<uint64_t>(i)), 1U));
	for (int i = N / 10; i < N * 5; ++i) {
		CHECK(t.set(scrambled(static_cast<uint64_t>(i)), 1U));
		CHECK(t.pop(scrambled(static_cast<uint64_t>(i - N / 10))));
	}
	sink = t.size();
	CHECK(t.cleanup());
}

NI static auto walk(const table<uint64_t, uint64_t>& t) -> void {
	uint64_t sum = 0;
	for (int r = 0; r < 10; ++r)
		t.each([&sum](const uint64_t, const uint64_t v) { sum += v; });
	sink = sum;
}

NI static auto small_maps(void) -> void {
	uint64_t sum = 0;
	for (int m = 0; m < N / 10; ++m) {
		table<int, int> t;
		for (int i = 0; i < 8; ++i)
			CHECK(t.set(i * 31 + m, i));
		for (int i = 0; i < 8; ++i)
			sum += static_cast<uint64_t>(*t.get(i * 31 + m));
		CHECK(t.cleanup());
	}
	sink = sum;
}

static char words[N / 10][24];

NI static auto text_keys(void) -> void {
	text_table t;
	for (int i = 0; i < N / 10; ++i)
		CHECK(t.set(words[i], i));
	uint64_t sum = 0;
	for (int r = 0; r < 10; ++r)
		for (int i = 0; i < N / 10; ++i)
			sum += static_cast<uint64_t>(*t.get(words[(i * 7919) % (N / 10)]));
	sink = sum;
	CHECK(t.cleanup());
}


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (!open_counters())
		return 1;
	for (int i = 0; i < N / 10; ++i)
		snprintf(words[i], sizeof words[i], "key-%llu", static_cast<unsigned long long>(scrambled(static_cast<uint64_t>(i)) % 100000000000U));
	bench("insert 1e6 scrambled u64, growing, then cleanup", insert_cleanup);
	table<uint64_t, uint64_t> t;
	for (int i = 0; i < N; ++i)
		CHECK(t.set(scrambled(static_cast<uint64_t>(i)), static_cast<uint64_t>(i)));
	bench("1e7 lookup hits in 1e6 entries", [&t] { lookup_hits(t); });
	bench("1e7 lookup misses", "misses rarely touch a slot", [&t] { lookup_misses(t); });
	bench("walk 1e6 entries 10 times", [&t] { walk(t); });
	CHECK(t.cleanup());
	bench("insert + pop churn, 1e5 live, 5e6 rounds", "DELETED reclaimed by in-place rehash", churn);
	bench("1e5 maps of 8 ints, built, read, destroyed", small_maps);
	bench("str keys, 1e5 inserted, 1e6 lookups by literal", "std: std::string keys", text_keys);
	return 0;
}
