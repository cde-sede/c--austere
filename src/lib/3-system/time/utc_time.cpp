#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/i64.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/time/impl/civil.hpp"
#include "3-system/time/utc_time.hpp"


static constexpr ca::i64 DAY = 86400;

[[nodiscard]] auto ca::sys::utc_time(const ca::i64 seconds) noexcept -> ca::result<ca::sys::calendar_time> {

	const ca::i64 days = (seconds >= 0 ? seconds : seconds - (DAY - 1)) / DAY;
	const ca::i64 in_day = seconds - days * DAY;
	const ca::sys::impl::civil_date date = ca::sys::impl::civil_from_days(days);
	if (date._year < -2147483648LL + 1900 || date._year > 2147483647LL) [[unlikely]]
		return ca::failure{ca::errc::out_of_range};
	const ca::i64 weekday = ((days + 4) % 7 + 7) % 7;
	const ca::i64 year_day = days - ca::sys::impl::days_from_civil(date._year, 1U, 1U) + 1;
	return ca::sys::calendar_time{
		static_cast<ca::i32>(date._year),
		static_cast<ca::i32>(date._month),
		static_cast<ca::i32>(date._day),
		static_cast<ca::i32>(in_day / 3600),
		static_cast<ca::i32>(in_day / 60 % 60),
		static_cast<ca::i32>(in_day % 60),
		static_cast<ca::i32>(weekday),
		static_cast<ca::i32>(year_day),
		0,
		false,
	};
}
