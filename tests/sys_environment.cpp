// expect: exit 0
// link: both
#include "3-system/environment/getenv.hpp"
#include "3-system/environment/setenv.hpp"
#include "3-system/environment/unsetenv.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/chdir.hpp"
#include "3-system/file/getcwd.hpp"

auto main(void) -> int {
	if (!ca::sys::setenv("CA_TEST_VAR", "one", true) || !ca::sys::setenv("CA_TEST_VAR", "two", false))
		return 1;
	auto value = ca::sys::getenv("CA_TEST_VAR");
	if (!value || *value != ca::sv::str{"one"})
		return 2;
	if (!ca::sys::unsetenv("CA_TEST_VAR") || ca::sys::getenv("CA_TEST_VAR") || !ca::sys::unsetenv("CA_TEST_VAR"))
		return 3;
	char buffer[2];
	auto small = ca::sys::getcwd(ca::span{buffer, 1U});
	if (small || small.error() != ca::errc::out_of_range)
		return 4;
	if (!ca::sys::chdir("/"))
		return 5;
	auto here = ca::sys::getcwd(ca::span{buffer, 2U});
	return here && *here == 1U && buffer[0] == '/' ? 0 : 6;
}
