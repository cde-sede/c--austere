// expect: error no matching
#include "2-vocabulary/realized.hpp"

auto main(void) -> int {
	const ca::realized<const int&> r{5};
	return *r;
}
