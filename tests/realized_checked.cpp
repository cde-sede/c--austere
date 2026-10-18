// expect: exit 42
#include "2-vocabulary/optional.hpp"

static auto twice(const ca::realized<int>& value) noexcept -> int {
	return *value * 2;
}

auto main(void) -> int {
	const ca::optional<int> o = 21;
	if (o)
		return twice(o);
	return 0;
}
