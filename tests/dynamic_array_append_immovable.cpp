// expect: error constraints not satisfied|no matching (member )?function
#include "5-collections/array/dynamic_array.hpp"

struct immovable {
	immovable(void) noexcept = default;
	immovable(immovable&&) = delete;
};

auto main(void) -> int {
	ca::dynamic_array<immovable> a;
	static_cast<void>(a.append(immovable{}));
	return 0;
}
