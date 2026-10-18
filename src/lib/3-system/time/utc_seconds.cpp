#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/time/utc_seconds.hpp"

#include <time.h>


[[nodiscard]] auto ca::sys::utc_seconds(const ca::sys::calendar_time& time) noexcept -> ca::result<ca::i64> {

	tm raw{};
	raw.tm_year = time.year() - 1900;
	 raw.tm_mon = time.month() - 1;
	raw.tm_mday = time.day();
	raw.tm_hour = time.hour();
	 raw.tm_min = time.minute();
	 raw.tm_sec = time.second();
	const time_t seconds = ::timegm(&raw);
	if (seconds == static_cast<time_t>(-1) && !(raw.tm_year == 69 && raw.tm_mon == 11 && raw.tm_mday == 31)) [[unlikely]]
		return ca::failure{ca::errc::out_of_range};
	return static_cast<ca::i64>(seconds);
}
