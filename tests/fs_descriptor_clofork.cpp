// expect: exit 0
#include "6-resources/fs/descriptor.hpp"

auto main(void) -> int {
	auto d = ca::fs::descriptor::open("/dev/null");
	if (!d)
		return 1;
	if constexpr (!ca::fs::has_clofork) {
		auto set = d->set(ca::fs::clofork | ca::fs::cloexec);
		if (set || set.error() != ca::errc::not_supported)
			return 2;
		auto untouched = d->test(ca::fs::cloexec);
		if (!untouched || *untouched)
			return 3;
		auto never = d->test(ca::fs::clofork);
		if (!never || *never)
			return 4;
		auto opened = ca::fs::descriptor::open("/dev/null", ca::fs::clofork);
		if (opened || opened.error() != ca::errc::not_supported)
			return 5;
	}
	return d->destroy() ? 0 : 6;
}
