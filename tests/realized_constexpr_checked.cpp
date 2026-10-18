// expect: exit 0
#include "2-vocabulary/result.hpp"

constexpr auto f(void) noexcept -> ca::result<int> {
	return 21;
}

constexpr auto g(void) noexcept -> int {
	const auto v = f();
	return v ? *v : 0;
}

static_assert(g() == 21);

auto main(void) -> int {
	return 0;
}
