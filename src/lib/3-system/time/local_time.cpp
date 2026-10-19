#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/time/impl/calendar_of.hpp"
#include "3-system/time/impl/time_zone.hpp"
#include "3-system/time/local_time.hpp"
#include "3-system/time/utc_time.hpp"

#include <time.h>


[[nodiscard]] auto ca::sys::local_time(const ca::i64 seconds) noexcept -> ca::result<ca::sys::calendar_time> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const ca::sys::impl::zone_offset zone = ca::sys::impl::zone_offset_at(seconds);
	ca::i64 shifted;
	if (__builtin_add_overflow(seconds, static_cast<ca::i64>(zone._offset), &shifted)) [[unlikely]]
		return ca::failure{ca::errc::out_of_range};
	const auto local = ca::sys::utc_time(shifted);
	if (!local) [[unlikely]]
		return local;
	return ca::sys::calendar_time{local->year(), local->month(), local->day(), local->hour(), local->minute(), local->second(), local->weekday(), local->year_day(), zone._offset, zone._daylight};
#else
	::tzset();
	const time_t moment = static_cast<time_t>(seconds);
	tm raw{};
	if (::localtime_r(&moment, &raw) == nullptr) [[unlikely]]
		return ca::failure{ca::errc::out_of_range};
	return ca::sys::impl::calendar_of(&raw);
#endif
}
