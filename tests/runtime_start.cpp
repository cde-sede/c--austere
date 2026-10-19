// expect: exit 0
// link: freestanding
#include "1-primitives/memory/string_length.hpp"
#include "3-system/environment/getenv.hpp"
#include "3-system/environment/setenv.hpp"
#include "3-system/environment/unsetenv.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/open.hpp"
#include "3-system/file/write.hpp"
#include "3-system/time/clock.hpp"
#include "3-system/time/now.hpp"

static int constructed = 0;

struct global final {
	int value;
	global(void) noexcept : value{42} {
		constructed = constructed + 1;
	}
};

static global object;
static thread_local int per_thread = 7;

auto main(int argc, char** argv) -> int {
	if (argc < 1 || argv[0] == nullptr || ca::string_length(argv[0]) == 0U)
		return 1;
	if (constructed != 1 || object.value != 42)
		return 2;
	per_thread = per_thread + 1;
	if (per_thread != 8)
		return 3;
	auto missing = ca::sys::open("/no/such/file", 0, 0U);
	if (missing || ca::sys::last_error() != ca::errc::no_entry)
		return 4;
	if (!ca::sys::getenv("PATH"))
		return 5;
	if (!ca::sys::setenv("CA_RUNTIME_TEST", "yes", true))
		return 6;
	auto value = ca::sys::getenv("CA_RUNTIME_TEST");
	if (!value || *value != ca::sv::str{"yes"})
		return 7;
	if (!ca::sys::setenv("CA_RUNTIME_TEST", "no", false))
		return 8;
	auto kept = ca::sys::getenv("CA_RUNTIME_TEST");
	if (!kept || *kept != ca::sv::str{"yes"})
		return 8;
	if (!ca::sys::unsetenv("CA_RUNTIME_TEST") || ca::sys::getenv("CA_RUNTIME_TEST"))
		return 9;
	if (ca::sys::setenv("BAD=NAME", "x", true))
		return 10;
	if (ca::sys::now(ca::sys::clock::realtime) < 1700000000LL * 1000000000LL)
		return 11;
	const char text[] = "";
	return ca::sys::write(1, ca::span<const ca::byte>{reinterpret_cast<const ca::byte*>(text), 0U}) ? 0 : 12;
}
