#ifndef ca_system_time_utc_seconds_hpp
#define ca_system_time_utc_seconds_hpp

#include "1-primitives/integers/i64.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/time/calendar_time.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- UTC SECONDS ----------------------------------------------------

		/* @DOC Seconds since the epoch of time read as UTC, from year to second only; fields out of range
		   carry over (month 13 is January next year). errc::out_of_range when it does not fit. */
		[[nodiscard]] auto utc_seconds(const ca::sys::calendar_time& time) noexcept -> ca::result<ca::i64>;

	} // namespace sys

} // namespace ca

#endif // ca_system_time_utc_seconds_hpp
