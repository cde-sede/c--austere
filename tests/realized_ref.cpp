// expect: exit 0
#include "2-vocabulary/optional.hpp"

struct point {
	int x;
};

static auto bump(const ca::realized<int&>& value) noexcept -> void {
	*value += 1;
}

auto main(void) -> int {
	int a = 1;
	int b = 2;
	ca::realized<int&> r{a};
	*r = 10;
	r = ca::realized<int&>{b};
	*r = 20;
	if (a != 10 || b != 20)
		return 1;
	point p{3};
	const ca::realized<point&> q{p};
	q->x = 4;
	if (p.x != 4)
		return 2;
	ca::optional<int&> o{a};
	if (!o)
		return 3;
	bump(o);
	return a == 11 ? 0 : 4;
}
