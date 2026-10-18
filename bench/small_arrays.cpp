// Benchmarks for ca::inline_array and ca::small_array; -DUSE_STD runs the same cases on
// std::vector, the std answer to "a short list of elements" (C++26 inplace_vector is not in the
// toolchains yet).

#include "measure.hpp"

#ifdef USE_STD
#include <utility>
#include <vector>

template <typename T, unsigned N>
struct fixed {
	std::vector<T> v;
	auto append(T x) -> bool { v.push_back(std::move(x)); return true; }
	auto pop(void) -> T { T x = std::move(v.back()); v.pop_back(); return x; }
	auto operator[](const size_t i) -> T& { return v[i]; }
	auto size(void) const -> size_t { return v.size(); }
	auto destroy(void) -> bool { std::vector<T>{}.swap(v); return true; }
};

template <typename T, unsigned N>
using small = fixed<T, N>;
#else
#include "5-collections/array/inline_array.hpp"
#include "5-collections/array/small_array.hpp"

template <typename T, unsigned N>
struct fixed {
	ca::inline_array<T, N> v;
	auto append(T x) -> bool { return static_cast<bool>(v.append(ca::move(x))); }
	auto pop(void) -> T { auto x = v.pop(); if (!x) __builtin_trap(); return ca::move(*x); }
	auto operator[](const size_t i) -> T& { return v[i]; }
	auto size(void) const -> size_t { return v.size(); }
	auto destroy(void) -> bool { return static_cast<bool>(v.destroy()); }
};

template <typename T, unsigned N>
struct small {
	ca::small_array<T, N> v;
	auto append(T x) -> bool { return static_cast<bool>(v.append(ca::move(x))); }
	auto pop(void) -> T { auto x = v.pop(); if (!x) __builtin_trap(); return ca::move(*x); }
	auto operator[](const size_t i) -> T& { return v[i]; }
	auto size(void) const -> size_t { return v.size(); }
	auto destroy(void) -> bool { return static_cast<bool>(v.destroy()); }
};
#endif

#define CHECK(x) do { if (!(x)) __builtin_trap(); } while (0)


// -- CASES ------------------------------------------------------------------

static const int N = 1000000;

template <typename A>
NI static auto build_drop(const int count) -> void {
	uint64_t sum = 0;
	for (int i = 0; i < N; ++i) {
		A a;
		for (int k = 0; k < count; ++k)
			CHECK(a.append(i + k));
		sum += static_cast<uint64_t>(a[a.size() - 1]);
		CHECK(a.destroy());
	}
	sink = sum;
}

template <typename A>
NI static auto churn(void) -> void {
	A a;
	uint64_t sum = 0;
	for (int i = 0; i < N * 10; ++i) {
		CHECK(a.append(i));
		if (a.size() == 8)
			while (a.size() != 0)
				sum += static_cast<uint64_t>(a.pop());
	}
	sink = sum;
	CHECK(a.destroy());
}


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (!open_counters())
		return 1;
	section("inline_array");
	bench("1e6 times build 4 elements and drop", [] { build_drop<fixed<int, 8>>(4); });
	bench("1e6 times build 8 elements and drop", [] { build_drop<fixed<int, 8>>(8); });
	bench("churn, 1e7 appends, popping all at 8", "gcc: vectorized pops stall on store forwarding", [] { churn<fixed<int, 8>>(); });
	section("small_array");
	bench("1e6 times build 4 elements (inline) and drop", [] { build_drop<small<int, 8>>(4); });
	bench("1e6 times build 32 elements (leaves the inline room) and drop", [] { build_drop<small<int, 8>>(32); });
	bench("churn, 1e7 appends, popping all at 8", "gcc: vectorized pops stall on store forwarding", [] { churn<small<int, 8>>(); });
	return 0;
}
