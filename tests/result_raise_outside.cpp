// expect: trap
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"

static auto failing(void) noexcept -> ca::result<void> {
	return ca::failure{ca::errc::exists};
}

auto main(void) -> int {
	failing().raise();
	return 0;
}
