// expect: exit 0
#include "2-vocabulary/fail.hpp"
#include "5-collections/array/slice.hpp"

static int clones = 0;
static int cleanups = 0;

struct fragile {
	int id;
	auto clone(void) const noexcept -> ca::result<fragile> {
		if (++clones == 3)
			return ca::fail(7);
		return fragile{id};
	}
	auto destroy(void) noexcept -> ca::result<void> {
		++cleanups;
		return ca::result<void>{};
	}
};

auto main(void) -> int {
	auto s = ca::slice<fragile>::make(4U, fragile{1});
	if (!s)
		return 1;
	auto c = s->clone();
	if (c || c.error().code() != 7 || cleanups != 2)
		return 2;
	return s->destroy() && cleanups == 6 ? 0 : 3;
}
