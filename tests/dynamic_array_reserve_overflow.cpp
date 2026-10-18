// expect: exit 0
#include "1-primitives/limits.hpp"
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	if (!a.append(1))
		return 1;
	auto r = a.reserve(ca::limits<ca::usz>::max());
	if (r)
		return 2;
	return a.size() == 1 && a[0] == 1 && a.capacity() == 4 ? 0 : 3;
}
