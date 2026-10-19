// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

struct tracked {
	static inline int alive = 0;
	int value;
	tracked(const int v) noexcept : value{v} { ++alive; }
	tracked(tracked&& other) noexcept : value{other.value} { ++alive; }
	~tracked(void) noexcept { --alive; }
};

auto main(void) -> int {
	ca::dynamic_array<int> a;
	if (a.swap_pop(0))
		return 1;
	for (int i = 0; i < 5; ++i) {
		if (!a.append(i))
			return 2;
	}
	auto r = a.swap_pop(1);
	if (!r || *r != 1 || a.size() != 4 || a[0] != 0 || a[1] != 4 || a[3] != 3)
		return 3;
	auto tail = a.swap_pop(3);
	if (!tail || *tail != 3 || a.size() != 3 || a[2] != 2)
		return 4;
	if (a.swap_pop(3))
		return 5;
	if (!a.cleanup())
		return 6;

	{
		ca::dynamic_array<tracked> t;
		for (int i = 0; i < 4; ++i) {
			if (!t.emplace(i))
				return 7;
		}
		auto first = t.swap_pop(0);
		if (!first || first->value != 0 || t[0].value != 3 || t[2].value != 2 || tracked::alive != 4)
			return 8;
		auto last = t.swap_pop(2);
		if (!last || last->value != 2 || t.size() != 2 || tracked::alive != 4)
			return 9;
		if (!t.cleanup())
			return 11;
	}
	return tracked::alive == 0 ? 0 : 10;
}
