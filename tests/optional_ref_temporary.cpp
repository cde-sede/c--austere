// expect: error no matching
#include "2-vocabulary/optional.hpp"

auto main(void) -> int {
	ca::optional<const int&> o{5};
	return o.has_value();
}
