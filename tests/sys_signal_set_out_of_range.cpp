// expect: trap
#include "3-system/process/signal_set.hpp"

auto main(int argc, char**) -> int {
	ca::sys::signal_set set;
	set.add(64 + argc);
	return set.empty() ? 0 : 1;
}
