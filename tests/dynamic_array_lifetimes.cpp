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

auto main(void) -> int {
	{
		ca::dynamic_array<tracked> a;
		for (int i = 0; i < 4; ++i) {
			if (!a.emplace(i))
				return 1;
		}
		if (!a.append(a[0]))
			return 2;
		if (a[4].value != 0 || tracked::alive != 5)
			return 3;
		if (!a.reserve(1000) || a.capacity() < 1000 || a[3].value != 3 || tracked::alive != 5)
			return 4;
		if (!a.clear() || tracked::alive != 0 || a.capacity() < 1000)
			return 5;
		if (!a.emplace(7))
			return 6;
	}
	return tracked::alive == 0 ? 0 : 7;
}
