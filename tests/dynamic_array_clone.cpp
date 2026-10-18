// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<int> a;
	for (int i = 0; i < 10; ++i) {
		if (!a.append(i))
			return 1;
	}
	auto c = a.clone();
	if (!c)
		return 2;
	if (c->size() != 10 || c->capacity() < 10 || (*c)[9] != 9 || c->data() == a.data())
		return 3;
	if (!c->destroy())
		return 4;
	return a.destroy() ? 0 : 5;
}
