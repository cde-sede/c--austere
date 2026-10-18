// expect: error not supported here
#include "6-resources/fs/descriptor.hpp"

struct missing_t final {
	using self = missing_t;
	using domain = ca::fs::descriptor_flags;
	static constexpr int VALUE = 0;
	static constexpr int OPEN_VALUE = 0;
	static constexpr bool SUPPORTED = false;
};

auto main(void) -> int {
	auto d = ca::fs::descriptor::open("/dev/null");
	if (!d)
		return 1;
	*d |= ca::option_set<missing_t>{};
	return 0;
}
