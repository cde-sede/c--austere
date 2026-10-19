// expect: exit 0
// link: libc
// ca::sys::local_time (own TZif and POSIX TZ reading on linux x86-64) against glibc's
// localtime_r, for TZ unset, empty, zone names (northern and southern daylight saving, half-
// and quarter-hour offsets, a zone without daylight saving), ":" names, absolute paths, POSIX
// rules, and an unknown name, at instants from 1900 to 2100 and around transitions.
#include "1-primitives/integers/i64.hpp"
#include "3-system/environment/setenv.hpp"
#include "3-system/environment/unsetenv.hpp"
#include "3-system/time/local_time.hpp"

namespace libc {

	struct tm final {
		int tm_sec;
		int tm_min;
		int tm_hour;
		int tm_mday;
		int tm_mon;
		int tm_year;
		int tm_wday;
		int tm_yday;
		int tm_isdst;
		long tm_gmtoff;
		const char* tm_zone;
	};

} // namespace libc

extern "C" auto localtime_r(const long* time, libc::tm* out) noexcept -> libc::tm*;
extern "C" auto tzset(void) noexcept -> void;

static auto agrees(const ca::i64 seconds) -> bool {
	libc::tm want{};
	const long moment = seconds;
	if (localtime_r(&moment, &want) == nullptr)
		return false;
	auto got = ca::sys::local_time(seconds);
	return got && got->year() == want.tm_year + 1900 && got->month() == want.tm_mon + 1 && got->day() == want.tm_mday
		&& got->hour() == want.tm_hour && got->minute() == want.tm_min && got->second() == want.tm_sec
		&& got->weekday() == want.tm_wday && got->year_day() == want.tm_yday + 1 && got->utc_offset() == want.tm_gmtoff
		&& got->daylight_saving() == (want.tm_isdst > 0);
}

static auto zone(const char* const tz) -> bool {
	if (tz == nullptr ? !ca::sys::unsetenv("TZ") : !ca::sys::setenv("TZ", tz, true))
		return false;
	tzset();
	for (ca::i64 s = -2208988800LL; s < 4102444800LL; s += 86400LL * 37 + 3601)
		if (!agrees(s))
			return false;
	for (ca::i64 year = 1970; year < 2060; ++year) {
		const ca::i64 start = (year - 1970) * 31556952LL;
		for (ca::i64 s = start + 86400LL * 60; s < start + 86400LL * 330; s += 1800)
			if (!agrees(s))
				return false;
		if (year > 1975)
			year += 7;
	}
	return true;
}

auto main(void) -> int {
	const char* const zones[] = {
		"Europe/Paris", "America/New_York", "Australia/Sydney", "Asia/Kolkata", "America/St_Johns", "Pacific/Chatham",
		"Asia/Tokyo", "UTC", "", ":Europe/London", "/usr/share/zoneinfo/America/Santiago", "EST5EDT,M3.2.0,M11.1.0",
		"CET-1CEST,M3.5.0,M10.5.0/3", "<+0330>-3:30", "XYZ3ABC", "Africa/Casablanca", "America/Sao_Paulo", "No/Such_Zone",
	};
	int index = 1;
	for (const char* const tz : zones) {
		if (!zone(tz))
			return index;
		++index;
	}
	return zone(nullptr) ? 0 : 99;
}
