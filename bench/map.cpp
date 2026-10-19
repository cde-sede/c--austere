// Benchmarks for ca::map; -DUSE_STD runs the same cases on std::map.

#include "measure.hpp"

#ifdef USE_STD
#include <functional>
#include <map>
#include <string>
#include <string_view>

template <typename K, typename V>
struct tree {
	std::map<K, V> m;
	auto set(K k, V v) -> bool { m[k] = v; return true; }
	auto get(const K& k) const -> const V* { auto it = m.find(k); return it == m.end() ? nullptr : &it->second; }
	auto pop(const K& k) -> bool { return m.erase(k) != 0; }
	auto ceiling(const K& k) const -> const V* { auto it = m.lower_bound(k); return it == m.end() ? nullptr : &it->second; }
	auto size(void) const -> size_t { return m.size(); }
	template <typename F> auto each(F f) const -> void { for (const auto& [k, v] : m) f(k, v); }
	auto cleanup(void) -> bool { m.clear(); return true; }
};

struct text_tree {
	std::map<std::string, int, std::less<>> m;
	auto set(const char* k, int v) -> bool { m[k] = v; return true; }
	auto get(const char* k) const -> const int* { auto it = m.find(std::string_view{k}); return it == m.end() ? nullptr : &it->second; }
	auto cleanup(void) -> bool { m.clear(); return true; }
};
#else
#include "5-collections/map/map.hpp"
#include "5-collections/string/str.hpp"

template <typename K, typename V>
struct tree {
	ca::map<K, V> m;
	auto set(K k, V v) -> bool { return static_cast<bool>(m.set(k, v)); }
	auto get(const K& k) const -> const V* { auto r = m.get(k); return r ? &*r : nullptr; }
	auto pop(const K& k) -> bool { return m.pop(k).has_value(); }
	auto ceiling(const K& k) const -> const V* { auto r = m.ceiling(k); return r ? &r->value : nullptr; }
	auto size(void) const -> size_t { return m.size(); }
	template <typename F> auto each(F f) const -> void { for (const auto [k, v] : m) f(k, v); }
	auto cleanup(void) -> bool { return static_cast<bool>(m.cleanup()); }
};

struct text_tree {
	ca::map<ca::str, int> m;
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
	tree<uint64_t, uint64_t> t;
	for (int i = 0; i < N; ++i)
		CHECK(t.set(scrambled(static_cast<uint64_t>(i)), static_cast<uint64_t>(i)));
	sink = t.size();
	CHECK(t.cleanup());
}

NI static auto insert_sorted(void) -> void {
	tree<uint64_t, uint64_t> t;
	for (int i = 0; i < N; ++i)
		CHECK(t.set(static_cast<uint64_t>(i), static_cast<uint64_t>(i)));
	sink = t.size();
	CHECK(t.cleanup());
}

NI static auto lookup_hits(const tree<uint64_t, uint64_t>& t) -> void {
	uint64_t sum = 0;
	for (int r = 0; r < 3; ++r)
		for (int i = 0; i < N; ++i)
			sum += *t.get(scrambled(static_cast<uint64_t>(i) * 7919U % N));
	sink = sum;
}

NI static auto ceilings(const tree<uint64_t, uint64_t>& t) -> void {
	uint64_t found = 0;
	for (int i = 0; i < N; ++i)
		found += t.ceiling(scrambled(static_cast<uint64_t>(N + i))) != nullptr ? 1U : 0U;
	sink = found;
}

NI static auto walk(const tree<uint64_t, uint64_t>& t) -> void {
	uint64_t sum = 0;
	for (int r = 0; r < 10; ++r)
		t.each([&sum](const uint64_t, const uint64_t v) { sum += v; });
	sink = sum;
}

NI static auto erase_all(void) -> void {
	tree<uint64_t, uint64_t> t;
	for (int i = 0; i < N; ++i)
		CHECK(t.set(scrambled(static_cast<uint64_t>(i)), 1U));
	for (int i = 0; i < N; ++i)
		CHECK(t.pop(scrambled(static_cast<uint64_t>(i) * 7919U % N)));
	sink = t.size();
	CHECK(t.cleanup());
}

static char words[N / 10][24];

NI static auto text_keys(void) -> void {
	text_tree t;
	for (int i = 0; i < N / 10; ++i)
		CHECK(t.set(words[i], i));
	uint64_t sum = 0;
	for (int r = 0; r < 5; ++r)
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
	bench("insert 1e6 scrambled u64, then cleanup", "entries shift within nodes", insert_cleanup);
	bench("insert 1e6 u64 in order", insert_sorted);
	tree<uint64_t, uint64_t> t;
	for (int i = 0; i < N; ++i)
		CHECK(t.set(scrambled(static_cast<uint64_t>(i)), static_cast<uint64_t>(i)));
	bench("3e6 lookup hits in 1e6 entries", [&t] { lookup_hits(t); });
	bench("1e6 ceiling misses", "std: lower_bound", [&t] { ceilings(t); });
	bench("walk 1e6 entries 10 times", [&t] { walk(t); });
	CHECK(t.cleanup());
	bench("insert 1e6, then pop all in scrambled order", erase_all);
	bench("str keys, 1e5 inserted, 5e5 lookups by literal", "std: std::less<> and string_view lookups", text_keys);
	return 0;
}
