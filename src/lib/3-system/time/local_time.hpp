#ifndef ca_system_time_local_time_hpp
#define ca_system_time_local_time_hpp

#include "1-primitives/integers/i64.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/time/calendar_time.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- LOCAL TIME -----------------------------------------------------

		/* @DOC Date and time of seconds since the epoch, in the local time zone (TZ, or
		   /etc/localtime, re-read when it changes). errc::out_of_range when the year does not
		   fit. */
		[[nodiscard]] auto local_time(ca::i64 seconds) noexcept -> ca::result<ca::sys::calendar_time>;

	} // namespace sys

} // namespace ca

#endif // ca_system_time_local_time_hpp
