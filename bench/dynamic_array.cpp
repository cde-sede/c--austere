// Benchmarks for ca::dynamic_array; -DUSE_STD runs the same cases on std::vector.
// variant: heap
// variant: libc -DBENCH_LIBC

#include "measure.hpp"

#ifdef USE_STD
#include <utility>
#include <vector>

template <typename T>
struct arr {
	std::vector<T> v;
	arr(void) = default;
	explicit arr(std::vector<T> x) : v(std::move(x)) {}
	auto append(T x) -> bool { v.push_back(std::move(x)); return true; }
	auto operator[](const size_t i) -> T& { return v[i]; }
	auto size(void) const -> size_t { return v.size(); }
	auto reserve(const size_t n) -> bool { v.reserve(n); return true; }
	auto insert(const size_t i, T x) -> bool { v.insert(v.begin() + static_cast<long>(i), std::move(x)); return true; }
	auto pop(const size_t i) -> bool { v.erase(v.begin() + static_cast<long>(i)); return true; }
	auto pop(void) -> bool { v.pop_back(); return true; }
	auto shrink(void) -> bool { v.shrink_to_fit(); return true; }
	auto resize(const size_t n) -> bool { v.resize(n); return true; }
	auto clone(void) const -> arr { return arr{v}; }
	auto cleanup(void) -> bool { std::vector<T>{}.swap(v); return true; }
	auto begin(void) -> T* { return v.data(); }
	auto end(void) -> T* { return v.data() + v.size(); }
};
#define CLONE(a) (a).clone()
#define OK(x) (x)
#define MOVE(x) std::move(x)
#else
#include "5-collections/array/dynamic_array.hpp"
#ifdef BENCH_LIBC
#include "4-lifecycle/memory/libc_allocator.hpp"
template <typename T>
using arr = ca::dynamic_array<T, ca::libc_allocator>;
#else
template <typename T>
using arr = ca::dynamic_array<T>;
#endif
#define CLONE(a) ca::move(*(a).clone())
#define OK(x) static_cast<bool>(x)
#define MOVE(x) ca::move(x)
#endif

#define CHECK(x) do { if (!OK(x)) __builtin_trap(); } while (0)


// -- CASES ------------------------------------------------------------------

static const int N = 10000000;

NI static auto append_int(void) -> void {
	arr<int> a;
	for (int i = 0; i < N; ++i)
		CHECK(a.append(i));
	sink = static_cast<uint64_t>(a[N - 1]);
	CHECK(a.cleanup());
}

NI static auto append_u64_reserved(void) -> void {
	arr<uint64_t> a;
	CHECK(a.reserve(N));
	for (int i = 0; i < N; ++i)
		CHECK(a.append(static_cast<uint64_t>(i)));
	sink = a[N - 1];
	CHECK(a.cleanup());
}

NI static auto append_nested(void) -> void {
	arr<arr<int>> a;
	for (int i = 0; i < N / 10; ++i) {
		arr<int> in;
		CHECK(in.append(i));
		CHECK(a.append(MOVE(in)));
	}
	sink = static_cast<uint64_t>(a[N / 10 - 1][0]);
	CHECK(a.cleanup());
}

NI static auto write_u64(arr<uint64_t>& a) -> void {
	for (size_t i = 0; i < a.size(); ++i)
		a[i] = i * 3;
	sink = a[5];
}

NI static auto sum_u64(arr<uint64_t>& a) -> void {
	uint64_t s = 0;
	for (size_t i = 0; i < a.size(); ++i)
		s += a[i];
	sink = s;
}

NI static auto range_sum_u64(arr<uint64_t>& a) -> void {
	uint64_t s = 0;
	for (const auto x : a)
		s += x;
	sink = s;
}

NI static auto write_int(arr<int>& a) -> void {
	for (size_t i = 0; i < a.size(); ++i)
		a[i] = static_cast<int>(i) * 3;
	sink = static_cast<uint64_t>(a[5]);
}

NI static auto clone_int(arr<int>& a) -> void {
	auto c = CLONE(a);
	sink = static_cast<uint64_t>(c[7]);
	CHECK(c.cleanup());
}

NI static auto clone_nested(arr<arr<int>>& a) -> void {
	auto c = a.clone();
#ifdef USE_STD
	sink = static_cast<uint64_t>(c[7][0]);
	CHECK(c.cleanup());
#else
	sink = static_cast<uint64_t>((*c)[7][0]);
	CHECK(c->cleanup());
#endif
}

NI static auto insert_front_int(void) -> void {
	arr<int> a;
	for (int i = 0; i < 20000; ++i)
		CHECK(a.insert(0, i));
	sink = static_cast<uint64_t>(a[0]);
	CHECK(a.cleanup());
}

NI static auto insert_front_nested(void) -> void {
	arr<arr<int>> a;
	for (int i = 0; i < 5000; ++i)
		CHECK(a.insert(0, arr<int>{}));
	CHECK(a.cleanup());
}

NI static auto pop_front_int(void) -> void {
	arr<int> a;
	for (int i = 0; i < 20000; ++i)
		CHECK(a.append(i));
	for (int i = 0; i < 20000; ++i)
		CHECK(a.pop(0));
	CHECK(a.cleanup());
}

NI static auto churn_int(void) -> void {
	arr<int> a;
	for (int r = 0; r < 100; ++r) {
		for (int i = 0; i < 100000; ++i)
			CHECK(a.append(i));
		for (int i = 0; i < 100000; ++i)
			CHECK(a.pop());
	}
	CHECK(a.cleanup());
}

NI static auto tiny_arrays(void) -> void {
	for (int i = 0; i < 1000000; ++i) {
		arr<int> t;
		for (int j = 0; j < 3; ++j)
			CHECK(t.append(j));
		sink = static_cast<uint64_t>(t[2]);
		CHECK(t.cleanup());
	}
}

NI static auto resize_int(void) -> void {
	arr<int> a;
	CHECK(a.resize(N));
	sink = static_cast<uint64_t>(a[N - 1]);
	CHECK(a.cleanup());
}


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (!open_counters())
		return 1;

	bench("append 1e7 ints, growing", "heap: page remap past 128 KiB, no copy; gcc's figure moves with loop placement", append_int);
	bench("append 1e7 u64, reserved", append_u64_reserved);
	bench("append 1e6 nested arrays", "trivially relocatable: growth is a memcpy", append_nested);
	bench("resize to 1e7 ints", resize_int);
	{
		arr<uint64_t> a;
		for (int i = 0; i < N; ++i)
			CHECK(a.append(static_cast<uint64_t>(i)));
		bench("index write, 1e7 u64", [&] { write_u64(a); });
		bench("index sum, 1e7 u64", "the bound check folds into the loop bound", [&] { sum_u64(a); });
		bench("range-for sum, 1e7 u64", [&] { range_sum_u64(a); });
		CHECK(a.cleanup());
	}
	{
		arr<int> a;
		for (int i = 0; i < N; ++i)
			CHECK(a.append(i));
		bench("index write, 1e7 ints", [&] { write_int(a); });
		bench("clone 1e7 ints", "one memcpy", [&] { clone_int(a); });
		CHECK(a.cleanup());
	}
	{
		arr<arr<int>> a;
		for (int i = 0; i < N / 10; ++i) {
			arr<int> in;
			CHECK(in.append(i));
			CHECK(a.append(MOVE(in)));
		}
		bench("clone 1e6 nested arrays", [&] { clone_nested(a); });
		CHECK(a.cleanup());
	}
	bench("insert 2e4 ints at the front", insert_front_int);
	bench("insert 5e3 nested arrays at the front", "relocation by memmove, no move constructor", insert_front_nested);
	bench("pop 2e4 ints from the front", pop_front_int);
	bench("append/pop churn, 1e5 ints x 100", "checked pop: one emptiness compare std skips", churn_int);
	bench("1e6 arrays of 3 ints", "one heap block each", tiny_arrays);
	return 0;
}
