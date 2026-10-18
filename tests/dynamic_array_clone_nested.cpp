// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

auto main(void) -> int {
	ca::dynamic_array<ca::dynamic_array<int>> outer;
	for (int i = 0; i < 3; ++i) {
		ca::dynamic_array<int> inner;
		if (!inner.resize(static_cast<ca::usz>(i + 1), i) || !outer.append(ca::move(inner)))
			return 1;
	}
	auto c = outer.clone();
	if (!c)
		return 2;
	if (c->size() != 3 || (*c)[2].size() != 3 || (*c)[2][2] != 2 || (*c)[2].data() == outer[2].data())
		return 3;
	if (!c->destroy())
		return 4;
	return outer.destroy() ? 0 : 5;
}
