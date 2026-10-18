#include "1-primitives/integers/i32.hpp"
#include "3-system/time/impl/calendar_of.hpp"

#include <time.h>


[[nodiscard]] auto ca::sys::impl::calendar_of(const void* const raw) noexcept -> ca::sys::calendar_time {

	const tm& time = *static_cast<const tm*>(raw);
	return ca::sys::calendar_time{
		static_cast<ca::i32>(time.tm_year) + 1900,
		static_cast<ca::i32>(time.tm_mon) + 1,
		static_cast<ca::i32>(time.tm_mday),
		static_cast<ca::i32>(time.tm_hour),
		static_cast<ca::i32>(time.tm_min),
		static_cast<ca::i32>(time.tm_sec),
		static_cast<ca::i32>(time.tm_wday),
		static_cast<ca::i32>(time.tm_yday) + 1,
		static_cast<ca::i32>(time.tm_gmtoff),
		time.tm_isdst > 0,
	};
}
