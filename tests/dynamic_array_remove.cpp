// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

struct tracked {
	static inline int alive = 0;
	int value;
	tracked(const int v) noexcept : value{v} { ++alive; }
	tracked(tracked&& other) noexcept : value{other.value} { ++alive; }
	~tracked(void) noexcept { --alive; }
	auto operator==(const tracked& other) const noexcept -> bool { return value == other.value; }
};

auto main(void) -> int {
	ca::dynamic_array<int> a;
	if (a.remove(0))
		return 1;
	const int values[6] = {4, 1, 2, 1, 3, 0};
	if (!a.extend(values))
		return 2;
	auto r = a.remove(1);
	if (!r || *r != 1 || a.size() != 5 || a[0] != 4 || a[1] != 2 || a[2] != 1)
		return 3;
	if (a.remove(9) || a.size() != 5)
		return 4;
	auto zero = a.remove(0);
	if (!zero || *zero != 0 || a.size() != 4 || a[3] != 3)
		return 5;
	if (!a.cleanup())
		return 6;

	{
		ca::dynamic_array<tracked> t;
		for (int i = 0; i < 4; ++i) {
			if (!t.emplace(i))
				return 7;
		}
		auto two = t.remove(tracked{2});
		if (!two || two->value != 2 || t.size() != 3 || t[2].value != 3 || tracked::alive != 4)
			return 8;
		if (!t.cleanup())
			return 10;
	}
	return tracked::alive == 0 ? 0 : 9;
}
