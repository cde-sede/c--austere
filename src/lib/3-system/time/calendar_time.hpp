#ifndef ca_system_time_calendar_time_hpp
#define ca_system_time_calendar_time_hpp

#include "1-primitives/integers/i32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- CALENDAR TIME --------------------------------------------------

		/* @DOC A moment as a date and time of day in some time zone, by utc_time or local_time. */
		class calendar_time final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::calendar_time;


				// -- private members ----------------------------------------

				ca::i32 _year;
				ca::i32 _month;
				ca::i32 _day;
				ca::i32 _hour;
				ca::i32 _minute;
				ca::i32 _second;
				ca::i32 _weekday;
				ca::i32 _year_day;
				ca::i32 _utc_offset;
				bool _daylight_saving;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr calendar_time(const ca::i32 year, const ca::i32 month, const ca::i32 day, const ca::i32 hour, const ca::i32 minute, const ca::i32 second, const ca::i32 weekday, const ca::i32 year_day, const ca::i32 utc_offset, const bool daylight_saving) noexcept
				: _year{year}, _month{month}, _day{day}, _hour{hour}, _minute{minute}, _second{second}, _weekday{weekday}, _year_day{year_day}, _utc_offset{utc_offset}, _daylight_saving{daylight_saving} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC Year, as 2026. */
				[[nodiscard]] constexpr auto year(void) const noexcept -> ca::i32 {
					return _year;
				}

				/* @DOC Month, 1..12. */
				[[nodiscard]] constexpr auto month(void) const noexcept -> ca::i32 {
					return _month;
				}

				/* @DOC Day of the month, 1..31. */
				[[nodiscard]] constexpr auto day(void) const noexcept -> ca::i32 {
					return _day;
				}

				/* @DOC Hour, 0..23. */
				[[nodiscard]] constexpr auto hour(void) const noexcept -> ca::i32 {
					return _hour;
				}

				/* @DOC Minute, 0..59. */
				[[nodiscard]] constexpr auto minute(void) const noexcept -> ca::i32 {
					return _minute;
				}

				/* @DOC Second, 0..60 (60 for a leap second). */
				[[nodiscard]] constexpr auto second(void) const noexcept -> ca::i32 {
					return _second;
				}

				/* @DOC Day of the week, 0..6, Sunday first. */
				[[nodiscard]] constexpr auto weekday(void) const noexcept -> ca::i32 {
					return _weekday;
				}

				/* @DOC Day of the year, 1..366. */
				[[nodiscard]] constexpr auto year_day(void) const noexcept -> ca::i32 {
					return _year_day;
				}

				/* @DOC Seconds east of UTC of the zone; 0 for utc_time. */
				[[nodiscard]] constexpr auto utc_offset(void) const noexcept -> ca::i32 {
					return _utc_offset;
				}

				/* @DOC Whether daylight saving time is in effect. */
				[[nodiscard]] constexpr auto daylight_saving(void) const noexcept -> bool {
					return _daylight_saving;
				}

		}; // class calendar_time

	} // namespace sys

} // namespace ca

#endif // ca_system_time_calendar_time_hpp
