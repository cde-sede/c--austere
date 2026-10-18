// expect: trap
#include "2-vocabulary/result.hpp"

constexpr auto f(void) noexcept -> ca::result<int> {
	return 21;
}

auto main(void) -> int {
	auto v = f();
	if (v)
		return v.error().code();
	return 0;
}
