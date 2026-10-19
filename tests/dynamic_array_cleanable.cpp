// expect: exit 0
#include "2-vocabulary/fail.hpp"
#include "5-collections/array/dynamic_array.hpp"

static int cleanups = 0;

struct cleanable {
	int code;
	auto cleanup(void) noexcept -> ca::result<void> {
		++cleanups;
		if (code != 0)
			return ca::fail(code);
		return ca::result<void>{};
	}
};

auto main(void) -> int {
	ca::dynamic_array<cleanable> a;
	if (!a.append(cleanable{0}) || !a.append(cleanable{5}) || !a.append(cleanable{6}))
		return 1;
	auto d = a.cleanup();
	if (d)
		return 2;
	return d.error().code() == 5 && cleanups == 3 && a.capacity() == 0 ? 0 : 3;
}
