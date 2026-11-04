// expect: exit 0
// link: freestanding
// ldflags: -Wl,--export-dynamic
// A program exporting its symbols makes ca's thread-local variables preemptible: the static-pie
// link leaves their GOT slots to R_X86_64_TPOFF64 relocations, which _start must apply before
// anything touches them (stdout, the heap, the last error, the trace).
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/open.hpp"
#include "3-system/trace/fail.hpp"
#include "3-system/trace/of.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "6-resources/print/print.hpp"

static constinit thread_local ca::usz own = 41U;

auto main(void) -> int {
	if (++own != 42U)
		return 1;
	ca::dynamic_array<int> numbers;
	for (int i = 0; i < 1000; ++i) {
		if (!numbers.append(i))
			return 2;
	}
	if (!ca::print("{} numbers, last {}\n", numbers.size(), numbers.data()[999]))
		return 3;
	auto opened = ca::sys::open("/ca/no/such/file", 0, 0U);
	if (opened || ca::sys::last_error() != ca::errc::no_entry)
		return 4;
	const auto failed = ca::trace::fail(ca::errc::busy, "exported");
	if (ca::trace::of(failed.error()).frames().size() != 1U)
		return 5;
	return numbers.cleanup() ? 0 : 6;
}
