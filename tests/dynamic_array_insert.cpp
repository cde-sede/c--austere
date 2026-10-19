// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

struct tracked {
	static inline int alive = 0;
	int value;
	tracked(const int v) noexcept : value{v} { ++alive; }
	tracked(tracked&& other) noexcept : value{other.value} { ++alive; }
	tracked(const tracked& other) noexcept : value{other.value} { ++alive; }
	~tracked(void) noexcept { --alive; }
};

template <typename T>
static auto check(void) noexcept -> int {
	ca::dynamic_array<T> a;
	if (!a.insert(0, 2) || !a.insert(0, 0) || !a.insert(1, 1) || !a.insert(3, 3))
		return 1;
	if (a.size() != 4 || a.capacity() != 4)
		return 2;
	if (!a.insert(2, a[3]))
		return 3;
	if (!a.insert(0, a[4]))
		return 4;
	const int expected[] = {3, 0, 1, 3, 2, 3};
	if (a.size() != 6)
		return 5;
	for (ca::usz i = 0; i < 6; ++i) {
		if (static_cast<int>(a[i]) != expected[i])
			return 6;
	}
	ca::dynamic_array<T> b;
	if (!b.emplace(7) || !b.emplace(8) || !a.reserve(8) || !a.insert(1, b.view()) || !a.insert(0, b.prefix(0U)))
		return 7;
	const int spliced[] = {3, 7, 8, 0, 1, 3, 2, 3};
	if (a.size() != 8)
		return 8;
	for (ca::usz i = 0; i < 8; ++i) {
		if (static_cast<int>(a[i]) != spliced[i])
			return 9;
	}
	ca::dynamic_array<T> c;
	const ca::usz room = a.capacity() - a.size() + 1U;
	for (ca::usz i = 0; i < room; ++i) {
		if (!c.emplace(9))
			return 10;
	}
	if (!a.insert(2, c.view()) || a.size() != 8 + room)
		return 11;
	if (static_cast<int>(a[1]) != 7 || static_cast<int>(a[2]) != 9 || static_cast<int>(a[1 + room]) != 9 || static_cast<int>(a[2 + room]) != 8
		|| static_cast<int>(a[7 + room]) != 3)
		return 12;
	return a.cleanup() && b.cleanup() && c.cleanup() ? 0 : 13;
}

struct tracked_int : tracked {
	using tracked::tracked;
	operator int(void) const noexcept { return value; }
};

auto main(void) -> int {
	if (const int r = check<int>())
		return r;
	if (const int r = check<tracked_int>())
		return 20 + r;
	return tracked::alive == 0 ? 0 : 40;
}
