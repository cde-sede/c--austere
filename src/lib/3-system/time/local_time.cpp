#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/time/impl/calendar_of.hpp"
#include "3-system/time/local_time.hpp"

#include <time.h>


[[nodiscard]] auto ca::sys::local_time(const ca::i64 seconds) noexcept -> ca::result<ca::sys::calendar_time> {

	::tzset();
	const time_t moment = static_cast<time_t>(seconds);
	tm raw{};
	if (::localtime_r(&moment, &raw) == nullptr) [[unlikely]]
		return ca::failure{ca::errc::out_of_range};
	return ca::sys::impl::calendar_of(&raw);
}
