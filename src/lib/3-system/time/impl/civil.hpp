#ifndef ca_system_time_impl_civil_hpp
#define ca_system_time_impl_civil_hpp

#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- CIVIL ----------------------------------------------------------

		namespace impl {

			/* @DOC A day of the proleptic Gregorian calendar: month 1 - 12, day 1 - 31. */
			struct civil_date final {
					using self = ca::sys::impl::civil_date;
					ca::i64 _year;
					ca::u32 _month;
					ca::u32 _day;
			}; // struct civil_date

			/* @DOC Days from 1970-01-01 to year-month-day (month 1 - 12, day 1 - 31), any year
			   (Howard Hinnant's days_from_civil: 400-year eras, March-based years). */
			[[nodiscard]] constexpr auto days_from_civil(ca::i64 year, const ca::u32 month, const ca::u32 day) noexcept -> ca::i64 {

				year -= month <= 2U ? 1 : 0;
				const ca::i64 era = (year >= 0 ? year : year - 399) / 400;
				const ca::u32 year_of_era = static_cast<ca::u32>(year - era * 400);
				const ca::u32 day_of_year = (153U * (month > 2U ? month - 3U : month + 9U) + 2U) / 5U + day - 1U;
				const ca::u32 day_of_era = year_of_era * 365U + year_of_era / 4U - year_of_era / 100U + day_of_year;
				return era * 146097 + static_cast<ca::i64>(day_of_era) - 719468;
			}

			/* @DOC The date days after 1970-01-01, any count (Howard Hinnant's civil_from_days). */
			[[nodiscard]] constexpr auto civil_from_days(ca::i64 days) noexcept -> ca::sys::impl::civil_date {

				days += 719468;
				const ca::i64 era = (days >= 0 ? days : days - 146096) / 146097;
				const ca::u32 day_of_era = static_cast<ca::u32>(days - era * 146097);
				const ca::u32 year_of_era = (day_of_era - day_of_era / 1460U + day_of_era / 36524U - day_of_era / 146096U) / 365U;
				const ca::u32 day_of_year = day_of_era - (365U * year_of_era + year_of_era / 4U - year_of_era / 100U);
				const ca::u32 shifted = (5U * day_of_year + 2U) / 153U;
				const ca::u32 day = day_of_year - (153U * shifted + 2U) / 5U + 1U;
				const ca::u32 month = shifted < 10U ? shifted + 3U : shifted - 9U;
				return {static_cast<ca::i64>(year_of_era) + era * 400 + (month <= 2U ? 1 : 0), month, day};
			}

			static_assert(ca::sys::impl::days_from_civil(1970, 1U, 1U) == 0);
			static_assert(ca::sys::impl::days_from_civil(2000, 3U, 1U) == 11017);
			static_assert(ca::sys::impl::civil_from_days(-1)._year == 1969);
			static_assert(ca::sys::impl::civil_from_days(11016)._day == 29);

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_time_impl_civil_hpp
