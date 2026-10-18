// expect: error indirection requires pointer|no match for .operator\*.
#include "2-vocabulary/result.hpp"

auto main(void) -> int {
	const ca::result<void> r;
	if (r)
		*r;
	return 0;
}
