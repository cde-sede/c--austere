// expect: trap
#include "2-vocabulary/optional.hpp"

constexpr auto f(void) noexcept -> ca::optional<int> {
	return ca::nullopt;
}

auto main(void) -> int {
	auto v = f();
	if (!v)
		return v.value();
	return 0;
}
