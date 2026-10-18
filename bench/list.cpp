// Benchmarks for ca::list; -DUSE_STD runs the same cases on std::list.

#include "measure.hpp"

#ifdef USE_STD
#include <iterator>
#include <list>
#include <utility>

template <typename T>
struct lst {
	std::list<T> l;
	using position = typename std::list<T>::iterator;
	auto append(T x) -> bool { l.push_back(std::move(x)); return true; }
	auto prepend(T x) -> bool { l.push_front(std::move(x)); return true; }
	auto take_front(void) -> T { T x = std::move(l.front()); l.pop_front(); return x; }
	auto insert(position at, T x) -> position { return l.insert(at, std::move(x)); }
	auto take(position at) -> T { T x = std::move(*at); l.erase(at); return x; }
	auto sort(void) -> void { l.sort(); }
	auto splice_all(lst& other) -> void { l.splice(l.end(), other.l); }
	auto begin(void) { return l.begin(); }
	auto end(void) { return l.end(); }
	auto size(void) const -> size_t { return l.size(); }
	auto destroy(void) -> bool { l.clear(); return true; }
};
#else
#include "5-collections/list/list.hpp"

template <typename T>
struct lst {
	ca::list<T> l;
	using position = typename ca::list<T>::iterator;
	auto append(T x) -> bool { return static_cast<bool>(l.append(ca::move(x))); }
	auto prepend(T x) -> bool { return static_cast<bool>(l.prepend(ca::move(x))); }
	auto take_front(void) -> T { auto x = l.pop_front(); if (!x) __builtin_trap(); return ca::move(*x); }
	auto insert(position at, T x) -> position { auto r = l.insert(at, ca::move(x)); if (!r) __builtin_trap(); return *r; }
	auto take(position at) -> T { auto x = l.pop(at); if (!x) __builtin_trap(); return ca::move(*x); }
	auto sort(void) -> void { l.sort(); }
	auto splice_all(lst& other) -> void { l.splice(l.end(), ca::move(other.l)); }
	auto begin(void) { return l.begin(); }
	auto end(void) { return l.end(); }
	auto size(void) const -> size_t { return l.size(); }
	auto destroy(void) -> bool { return static_cast<bool>(l.destroy()); }
};
#endif

#define CHECK(x) do { if (!(x)) __builtin_trap(); } while (0)


// -- CASES ------------------------------------------------------------------

static const int N = 1000000;

static auto scrambled(const int i) -> int {
	return static_cast<int>((static_cast<unsigned>(i) * 2654435761U) >> 8U);
}

NI static auto append_destroy(void) -> void {
	lst<int> l;
	for (int i = 0; i < N; ++i)
		CHECK(l.append(i));
	sink = l.size();
	CHECK(l.destroy());
}

NI static auto prepend_take(void) -> void {
	lst<int> l;
	uint64_t sum = 0;
	for (int i = 0; i < N; ++i) {
		CHECK(l.prepend(i));
		CHECK(l.prepend(i + 1));
		sum += static_cast<uint64_t>(l.take_front());
	}
	sink = sum;
	CHECK(l.destroy());
}

NI static auto walk(lst<int>& l) -> void {
	uint64_t sum = 0;
	for (int r = 0; r < 10; ++r)
		for (const int x : l)
			sum += static_cast<uint64_t>(x);
	sink = sum;
}

NI static auto sort_scrambled(void) -> void {
	lst<int> l;
	for (int i = 0; i < N; ++i)
		CHECK(l.append(scrambled(i)));
	l.sort();
	sink = static_cast<uint64_t>(*l.begin());
	CHECK(l.destroy());
}

NI static auto middle_edits(void) -> void {
	lst<int> l;
	for (int i = 0; i < 1000; ++i)
		CHECK(l.append(i));
	auto at = l.begin();
	for (int i = 0; i < 500; ++i)
		++at;
	uint64_t sum = 0;
	for (int i = 0; i < N * 10; ++i) {
		auto added = l.insert(at, i);
		sum += static_cast<uint64_t>(l.take(added));
	}
	sink = sum;
	CHECK(l.destroy());
}

NI static auto splice_runs(void) -> void {
	lst<int> all;
	for (int r = 0; r < N / 100; ++r) {
		lst<int> run;
		for (int i = 0; i < 100; ++i)
			CHECK(run.append(i));
		all.splice_all(run);
	}
	sink = all.size();
	CHECK(all.destroy());
}


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (!open_counters())
		return 1;
	bench("append 1e6, then destroy", append_destroy);
	bench("prepend twice, pop_front once, 1e6 times", prepend_take);
	lst<int> l;
	for (int i = 0; i < N; ++i)
		CHECK(l.append(i));
	bench("walk 1e6 nodes 10 times", "the heap packs nodes of one size class together", [&l] { walk(l); });
	CHECK(l.destroy());
	bench("sort 1e6 scrambled ints", sort_scrambled);
	bench("insert + pop at a held middle position, 1e7", middle_edits);
	bench("build 1e4 runs of 100 and splice each", "splice is O(1) in both; the gap is node allocation", splice_runs);
	return 0;
}
