// Benchmarks for ca::bit_array and ca::dynamic_bit_array; -DUSE_STD runs the same cases on
// std::bitset and std::vector<bool> (libstdc++'s _Find_first/_Find_next extensions for search).

#include "measure.hpp"

static const size_t BITS = 1U << 16U;
static const int ROUNDS = 2000;

#ifdef USE_STD
#include <bitset>
#include <vector>

using fixed = std::bitset<BITS>;

static auto set_bit(fixed& b, const size_t i) -> void { b.set(i); }
static auto first_set(const fixed& b) -> size_t { return b._Find_first(); }
static auto next_set(const fixed& b, const size_t i) -> size_t { return b._Find_next(i); }
static auto popcount_of(const fixed& b) -> size_t { return b.count(); }

struct growable {
	std::vector<bool> v;
	auto append(const bool x) -> bool { v.push_back(x); return true; }
	auto count(void) const -> size_t { size_t n = 0; for (const bool x : v) n += x; return n; }
	auto test(const size_t i) const -> bool { return v[i]; }
	auto size(void) const -> size_t { return v.size(); }
	auto cleanup(void) -> bool { std::vector<bool>{}.swap(v); return true; }
};
#else
#include "5-collections/array/bit_array.hpp"
#include "5-collections/array/dynamic_bit_array.hpp"

using fixed = ca::bit_array<BITS>;

static auto set_bit(fixed& b, const size_t i) -> void { b.set(i); }
static auto first_set(const fixed& b) -> size_t { return b.find_first().value_or(BITS); }
static auto next_set(const fixed& b, const size_t i) -> size_t { return b.find_next(i + 1U).value_or(BITS); }
static auto popcount_of(const fixed& b) -> size_t { return b.count(); }

struct growable {
	ca::dynamic_bit_array<> v;
	auto append(const bool x) -> bool { return static_cast<bool>(v.append(x)); }
	auto count(void) const -> size_t { return v.count(); }
	auto test(const size_t i) const -> bool { return v[i]; }
	auto size(void) const -> size_t { return v.size(); }
	auto cleanup(void) -> bool { return static_cast<bool>(v.cleanup()); }
};
#endif

#define CHECK(x) do { if (!(x)) __builtin_trap(); } while (0)


// -- CASES ------------------------------------------------------------------

static fixed sparse;

NI static auto count_fixed(void) -> void {
	size_t sum = 0;
	for (int r = 0; r < ROUNDS; ++r)
		sum += popcount_of(sparse);
	sink = sum;
}

NI static auto scan_fixed(void) -> void {
	size_t sum = 0;
	for (int r = 0; r < ROUNDS / 10; ++r)
		for (size_t i = first_set(sparse); i < BITS; i = next_set(sparse, i))
			sum += i;
	sink = sum;
}

NI static auto set_fixed(void) -> void {
	fixed b;
	for (int r = 0; r < ROUNDS; ++r)
		for (size_t i = static_cast<size_t>(r) & 63U; i < BITS; i += 61U)
			set_bit(b, i);
	sink = popcount_of(b);
}

NI static auto append_growable(void) -> void {
	growable g;
	for (size_t i = 0; i < BITS * 100U; ++i)
		CHECK(g.append((i * 2654435761U >> 7U) & 1U));
	sink = g.size();
	CHECK(g.cleanup());
}

NI static auto count_growable(growable& g) -> void {
	size_t sum = 0;
	for (int r = 0; r < 20; ++r)
		sum += g.count();
	sink = sum;
}

NI static auto test_growable(growable& g) -> void {
	size_t sum = 0;
	for (int r = 0; r < 5; ++r)
		for (size_t i = 0; i < g.size(); ++i)
			sum += g.test(i);
	sink = sum;
}


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (!open_counters())
		return 1;
	for (size_t i = 0; i < BITS; i += 97U)
		set_bit(sparse, i);
	section("bit_array");
	bench("count 64 Ki bits, 2e3 times", count_fixed);
	bench("scan a sparse set with find_first/find_next, 2e2 times", "std: libstdc++'s _Find_first/_Find_next", scan_fixed);
	bench("set every 61st bit, 2e3 times", set_fixed);
	section("dynamic_bit_array");
	bench("append 6.5e6 bits", append_growable);
	growable g;
	for (size_t i = 0; i < BITS * 100U; ++i)
		CHECK(g.append((i * 2654435761U >> 7U) & 1U));
	bench("count 6.5e6 bits, 20 times", "popcount per word against a bit loop", [&g] { count_growable(g); });
	bench("operator[] over 6.5e6 bits, 5 times", [&g] { test_growable(g); });
	CHECK(g.cleanup());
	return 0;
}
