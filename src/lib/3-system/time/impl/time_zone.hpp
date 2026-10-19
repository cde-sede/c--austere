#ifndef ca_system_time_impl_time_zone_hpp
#define ca_system_time_impl_time_zone_hpp

#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/i64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TIME ZONE ------------------------------------------------------

		namespace impl {

			/* @DOC How local time differs from UTC at some instant: seconds east, and whether
			   daylight saving is in effect. */
			struct zone_offset final {
					using self = ca::sys::impl::zone_offset;
					ca::i32 _offset;
					bool _daylight;
			}; // struct zone_offset

			/* @DOC The local zone's offset at seconds since the epoch, as glibc's localtime_r
			   finds it: TZ unset is /etc/localtime, empty is UTC, a leading ':' is dropped; a
			   TZ naming a TZif file (absolute, or under /usr/share/zoneinfo) uses its
			   transitions, then its POSIX footer past the last one; otherwise TZ is read as a
			   POSIX rule ("EST5EDT,M3.2.0,M11.1.0"), and UTC when it is not one either. The
			   zone is read once per TZ value and kept. Thread-safe. */
			[[nodiscard]] auto zone_offset_at(ca::i64 seconds) noexcept -> ca::sys::impl::zone_offset;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_time_impl_time_zone_hpp
