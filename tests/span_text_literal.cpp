// expect: error no matching|no viable|could not convert|conversion from
#include "2-vocabulary/span.hpp"

auto main(void) -> int {
	const ca::span<const ca::byte> bytes = "with its NUL";
	return static_cast<int>(bytes.size());
}
