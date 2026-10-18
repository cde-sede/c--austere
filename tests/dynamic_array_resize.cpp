// expect: exit 0
#include "2-vocabulary/fail.hpp"
#include "5-collections/array/dynamic_array.hpp"

struct cleanable {
	int code;
	auto destroy(void) noexcept -> ca::result<void> {
		if (code != 0)
			return ca::fail(code);
		return ca::result<void>{};
	}
};

struct tracked {
	static inline int alive = 0;
	int value;
	tracked(void) noexcept : value{-1} { ++alive; }
	tracked(const int v) noexcept : value{v} { ++alive; }
	tracked(tracked&& other) noexcept : value{other.value} { ++alive; }
	tracked(const tracked& other) noexcept : value{other.value} { ++alive; }
	~tracked(void) noexcept { --alive; }
};

auto main(void) -> int {
	ca::dynamic_array<int> a;
	if (!a.resize(3, 9) || a.size() != 3 || a.capacity() < 3 || a[2] != 9)
		return 1;
	if (!a.resize(5) || a[3] != 0 || a[4] != 0)
		return 2;
	if (!a.resize(1) || a.size() != 1 || a.capacity() < 5)
		return 3;
	if (!a.destroy())
		return 4;

	{
		ca::dynamic_array<tracked> t;
		if (!t.emplace(4) || !t.resize(9, t[0]) || t[8].value != 4 || tracked::alive != 9)
			return 5;
		if (!t.resize(10) || t[9].value != -1 || !t.resize(2) || tracked::alive != 2)
			return 6;
	}
	if (tracked::alive != 0)
		return 7;

	ca::dynamic_array<cleanable> c;
	if (!c.append(cleanable{0}) || !c.append(cleanable{3}) || !c.append(cleanable{4}))
		return 8;
	auto r = c.resize(1);
	if (r || r.error().code() != 3 || c.size() != 1)
		return 9;
	return c.destroy() ? 0 : 10;
}
