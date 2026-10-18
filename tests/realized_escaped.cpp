// expect: trap
// mode: debug
#include "2-vocabulary/result.hpp"

constexpr auto f(void) noexcept -> ca::result<int> {
	return 21;
}

auto main(void) -> int {
	auto v = f();
	__asm__ __volatile__("" : : "r"(&v) : "memory");
	return *v;
}
