// expect: exit 0
#include "2-vocabulary/fail.hpp"
#include "5-collections/array/dynamic_array.hpp"

static int clones = 0;
static int cleanups = 0;

struct fragile {
	int id;
	auto clone(void) const noexcept -> ca::result<fragile> {
		if (++clones == 3)
			return ca::fail(7);
		return fragile{id};
	}
	auto cleanup(void) noexcept -> ca::result<void> {
		++cleanups;
		return ca::result<void>{};
	}
};

auto main(void) -> int {
	ca::dynamic_array<fragile> a;
	for (int i = 0; i < 4; ++i) {
		if (!a.append(fragile{i}))
			return 1;
	}
	auto c = a.clone();
	if (c || c.error().code() != 7 || cleanups != 2)
		return 2;
	return a.cleanup() && cleanups == 6 ? 0 : 3;
}
