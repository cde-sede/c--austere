// expect: error no matching|no viable|could not convert|conversion from
#include "2-vocabulary/span.hpp"
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	const ca::span<int> dangling = ca::dynamic_array<int>{};
	return static_cast<int>(dangling.size());
}
