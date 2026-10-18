// expect: error signal_number_out_of_range|constant expression
#include "3-system/process/signal_set.hpp"

constexpr ca::sys::signal_set zero = [] {
	ca::sys::signal_set set;
	set.add(0);
	return set;
}();

auto main(void) -> int {
	return zero.empty() ? 0 : 1;
}
