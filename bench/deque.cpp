// Benchmarks for ca::deque; -DUSE_STD runs the same cases on std::deque.

#include "measure.hpp"

#ifdef USE_STD
#include <deque>
#include <utility>

template <typename T>
struct dq {
	std::deque<T> d;
	auto append(T x) -> bool { d.push_back(std::move(x)); return true; }
	auto prepend(T x) -> bool { d.push_front(std::move(x)); return true; }
	auto take_front(void) -> T { T x = std::move(d.front()); d.pop_front(); return x; }
	auto take_back(void) -> T { T x = std::move(d.back()); d.pop_back(); return x; }
	auto operator[](const size_t i) -> T& { return d[i]; }
	auto size(void) const -> size_t { return d.size(); }
	auto begin(void) { return d.begin(); }
	auto end(void) { return d.end(); }
	auto destroy(void) -> bool { std::deque<T>{}.swap(d); return true; }
};
#else
#include "5-collections/array/deque.hpp"

template <typename T>
struct dq {
	ca::deque<T> d;
	auto append(T x) -> bool { return static_cast<bool>(d.append(ca::move(x))); }
	auto prepend(T x) -> bool { return static_cast<bool>(d.prepend(ca::move(x))); }
	auto take_front(void) -> T { auto x = d.pop_front(); if (!x) __builtin_trap(); return ca::move(*x); }
	auto take_back(void) -> T { auto x = d.pop(); if (!x) __builtin_trap(); return ca::move(*x); }
	auto operator[](const size_t i) -> T& { return d[i]; }
	auto size(void) const -> size_t { return d.size(); }
	auto begin(void) { return d.begin(); }
	auto end(void) { return d.end(); }
	auto destroy(void) -> bool { return static_cast<bool>(d.destroy()); }
};
#endif

#define CHECK(x) do { if (!(x)) __builtin_trap(); } while (0)


// -- CASES ------------------------------------------------------------------

static const int N = 10000000;

NI static auto append_int(void) -> void {
	dq<int> d;
	for (int i = 0; i < N; ++i)
		CHECK(d.append(i));
	sink = static_cast<uint64_t>(d[N - 1]);
	CHECK(d.destroy());
}

NI static auto prepend_int(void) -> void {
	dq<int> d;
	for (int i = 0; i < N; ++i)
		CHECK(d.prepend(i));
	sink = static_cast<uint64_t>(d[N - 1]);
	CHECK(d.destroy());
}

NI static auto fifo_int(void) -> void {
	dq<int> d;
	for (int i = 0; i < 1000; ++i)
		CHECK(d.append(i));
	uint64_t sum = 0;
	for (int i = 0; i < N; ++i) {
		CHECK(d.append(i));
		sum += static_cast<uint64_t>(d.take_front());
	}
	sink = sum;
	CHECK(d.destroy());
}

NI static auto both_ends_u64(void) -> void {
	dq<uint64_t> d;
	for (int i = 0; i < 1000; ++i)
		CHECK(d.append(static_cast<uint64_t>(i)));
	uint64_t sum = 0;
	for (int i = 0; i < N; ++i) {
		if (i & 1) {
			CHECK(d.prepend(static_cast<uint64_t>(i)));
			sum += d.take_back();
		}
		else {
			CHECK(d.append(static_cast<uint64_t>(i)));
			sum += d.take_front();
		}
	}
	sink = sum;
	CHECK(d.destroy());
}

NI static auto index_sum(dq<int>& d) -> void {
	uint64_t sum = 0;
	for (int r = 0; r < 10; ++r)
		for (size_t i = 0; i < d.size(); ++i)
			sum += static_cast<uint64_t>(d[i]);
	sink = sum;
}

NI static auto range_sum(dq<int>& d) -> void {
	uint64_t sum = 0;
	for (int r = 0; r < 10; ++r)
		for (const int x : d)
			sum += static_cast<uint64_t>(x);
	sink = sum;
}


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (!open_counters())
		return 1;
	bench("append 1e7 ints, growing", append_int);
	bench("prepend 1e7 ints, growing", prepend_int);
	bench("FIFO 1e3 deep, 1e7 append + pop_front", fifo_int);
	bench("both ends, 1e7 u64", both_ends_u64);
	dq<int> d;
	for (int i = 0; i < 1000000; ++i)
		CHECK(i & 1 ? d.append(i) : d.prepend(i));
	bench("operator[] sum, 1e6 ints x 10", "one mask per index; std: a block lookup", [&d] { index_sum(d); });
	bench("range-for sum, 1e6 ints x 10", [&d] { range_sum(d); });
	CHECK(d.destroy());
	return 0;
}
