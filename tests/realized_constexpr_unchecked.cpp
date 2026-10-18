// expect: error value_accessed_without_a_check
// mode: debug
#include "2-vocabulary/result.hpp"

constexpr auto f(void) noexcept -> ca::result<int> {
	return 21;
}

static_assert(*f() == 21);

auto main(void) -> int {
	return 0;
}
