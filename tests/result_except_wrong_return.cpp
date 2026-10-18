// expect: error f must return the same ca::result
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"

auto main(void) -> int {
	ca::result<int> r{ca::failure{ca::errc::no_entry}};
	auto handled = r.except(ca::errc::no_entry, [](ca::error) noexcept { return ca::result<long>{0L}; });
	return handled ? 0 : 1;
}
