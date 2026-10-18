// expect: exit 0
#include "2-vocabulary/optional.hpp"

auto main(void) -> int {
	int x = 1;
	int y = 2;
	ca::optional<int&> o{x};
	o = y;
	if (!o || x != 1 || &*o != &y)
		return 1;
	ca::optional<int&> other{x};
	o = other;
	if (!o || y != 2 || &*o != &x)
		return 2;
	*o = 10;
	return x == 10 ? 0 : 3;
}
