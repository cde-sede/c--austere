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
	if (a.pop(0))
		return 1;
	for (int i = 0; i < 5; ++i) {
		if (!a.append(i))
			return 2;
	}
	auto r = a.pop(1);
	if (!r || *r != 1 || a.size() != 4 || a[1] != 2 || a[3] != 4)
		return 3;
	if (a.pop(4))
		return 4;
	if (!a.destroy())
		return 5;

	{
		ca::dynamic_array<tracked> t;
		for (int i = 0; i < 4; ++i) {
			if (!t.emplace(i))
				return 6;
		}
		auto first = t.pop(0);
		if (!first || first->value != 0 || t[0].value != 1 || t[2].value != 3 || tracked::alive != 4)
			return 7;
		auto last = t.pop(2);
		if (!last || last->value != 3 || t.size() != 2)
			return 8;
	}
	return tracked::alive == 0 ? 0 : 9;
}
