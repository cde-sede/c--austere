#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/time/impl/civil.hpp"
#include "3-system/time/utc_seconds.hpp"


/* As timegm: fields out of their range carry over (month 13 is January of the next year, day 0
   the last of the previous month). Never fails for i32 fields. */
[[nodiscard]] auto ca::sys::utc_seconds(const ca::sys::calendar_time& time) noexcept -> ca::result<ca::i64> {

	const ca::i64 months = static_cast<ca::i64>(time.month()) - 1;
	const ca::i64 carried = (months >= 0 ? months : months - 11) / 12;
	const ca::i64 year = static_cast<ca::i64>(time.year()) + carried;
	const ca::u32 month = static_cast<ca::u32>(months - carried * 12) + 1U;
	const ca::i64 days = ca::sys::impl::days_from_civil(year, month, 1U) + static_cast<ca::i64>(time.day()) - 1;
	return days * 86400 + static_cast<ca::i64>(time.hour()) * 3600 + static_cast<ca::i64>(time.minute()) * 60 + static_cast<ca::i64>(time.second());
}
