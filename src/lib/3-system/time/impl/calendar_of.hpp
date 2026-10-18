#ifndef ca_system_time_impl_calendar_of_hpp
#define ca_system_time_impl_calendar_of_hpp

#include "3-system/time/calendar_time.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- CALENDAR OF ----------------------------------------------------

		namespace impl {

			/* @DOC calendar_time of struct tm raw. */
			[[nodiscard]] auto calendar_of(const void* raw) noexcept -> ca::sys::calendar_time;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_time_impl_calendar_of_hpp
