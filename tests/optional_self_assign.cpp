// expect: exit 0
#include "2-vocabulary/optional.hpp"

struct tracked {
	static inline int alive = 0;
	static inline int assigned = 0;
	int value;
	tracked(const int v) noexcept : value{v} { ++alive; }
	tracked(const tracked& other) noexcept : value{other.value} { ++alive; }
	tracked(tracked&& other) noexcept : value{other.value} { ++alive; }
	~tracked(void) noexcept { --alive; }
	auto operator=(const tracked& other) noexcept -> tracked& { value = other.value; ++assigned; return *this; }
	auto operator=(tracked&& other) noexcept -> tracked& { value = other.value; ++assigned; return *this; }
};

auto main(void) -> int {
	ca::optional<int> o1{};
	ca::optional<int> o2{42};

	if (o2) {
		if (*o2 != 42)
			return 1;
	}
	else
		return 2;

	auto& a1 = o1;
	o1 = a1;
	if (o1)
		return 3;
	o1 = ca::move(a1);
	if (o1)
		return 4;

	auto& a2 = o2;
	o2 = a2;
	if (!o2 || *o2 != 42)
		return 5;
	o2 = ca::move(a2);
	if (!o2 || *o2 != 42)
		return 6;
	o2 = *o2;
	if (!o2 || *o2 != 42)
		return 7;

	{
		ca::optional<tracked> t{7};
		auto& alias = t;
		t = alias;
		t = ca::move(alias);
		if (!t || t->value != 7 || tracked::alive != 1 || tracked::assigned != 0)
			return 8;
		ca::optional<tracked> empty{};
		auto& none = empty;
		empty = none;
		if (empty || tracked::alive != 1)
			return 9;
	}
	return tracked::alive == 0 ? 0 : 10;
}
