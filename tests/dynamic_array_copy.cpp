// expect: error deleted (constructor|function)
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	auto b = a;
	return 0;
}
