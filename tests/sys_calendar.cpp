// expect: exit 0
// link: both
#include "3-system/environment/setenv.hpp"
#include "3-system/time/clock.hpp"
#include "3-system/time/local_time.hpp"
#include "3-system/time/now.hpp"
#include "3-system/time/resolution.hpp"
#include "3-system/time/utc_seconds.hpp"
#include "3-system/time/utc_time.hpp"

auto main(void) -> int {
	const ca::i64 step = ca::sys::resolution(ca::sys::clock::monotonic);
	if (step <= 0 || step > 1000000000)
		return 1;

	auto epoch = ca::sys::utc_time(0);
	if (!epoch || epoch->year() != 1970 || epoch->month() != 1 || epoch->day() != 1 || epoch->hour() != 0)
		return 2;
	if (epoch->weekday() != 4 || epoch->year_day() != 1 || epoch->utc_offset() != 0 || epoch->daylight_saving())
		return 3;

	auto leap = ca::sys::utc_time(951825599);
	if (!leap || leap->year() != 2000 || leap->month() != 2 || leap->day() != 29 || leap->hour() != 11
		|| leap->minute() != 59 || leap->second() != 59 || leap->year_day() != 60 || leap->weekday() != 2)
		return 4;
	auto back = ca::sys::utc_seconds(*leap);
	if (!back || *back != 951825599)
		return 5;
	const ca::sys::calendar_time overflow{2025, 13, 1, 0, 0, 0, 0, 0, 0, false};
	auto carried = ca::sys::utc_seconds(overflow).and_then([](const ca::i64 seconds) {
		return ca::sys::utc_time(seconds);
	});
	if (!carried || carried->year() != 2026 || carried->month() != 1)
		return 6;
	const ca::sys::calendar_time last_second{1969, 12, 31, 23, 59, 59, 0, 0, 0, false};
	auto minus_one = ca::sys::utc_seconds(last_second);
	if (!minus_one || *minus_one != -1)
		return 7;

	if (!ca::sys::setenv("TZ", "UTC0", true))
		return 8;
	const ca::i64 now = ca::sys::now(ca::sys::clock::realtime) / 1000000000;
	auto local = ca::sys::local_time(now);
	auto utc = ca::sys::utc_time(now);
	if (!local || !utc || local->utc_offset() != 0 || local->hour() != utc->hour() || local->day() != utc->day())
		return 9;
	if (!ca::sys::setenv("TZ", "EST5", true))
		return 10;
	auto east = ca::sys::local_time(0);
	if (!east || east->utc_offset() != -5 * 3600 || east->year() != 1969 || east->hour() != 19)
		return 11;
	return 0;
}
