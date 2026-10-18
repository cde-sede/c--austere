// expect: exit 5
#include "2-vocabulary/fail.hpp"
#include "2-vocabulary/result.hpp"

constexpr auto f(void) noexcept -> ca::result<int> {
	return ca::fail(5);
}

auto main(void) -> int {
	auto v = f();
	if (!v)
		return v.error().code();
	return 0;
}
