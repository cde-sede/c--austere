#ifndef ca_system_time_now_hpp
#define ca_system_time_now_hpp

#include "1-primitives/integers/i64.hpp"
#include "3-system/time/clock.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- NOW ------------------------------------------------------------

		/* @DOC Time of source in nanoseconds: since the epoch for realtime, since an arbitrary fixed
		   point for the others. Never fails. */
		[[nodiscard]] auto now(ca::sys::clock source) noexcept -> ca::i64;

	} // namespace sys

} // namespace ca

#endif // ca_system_time_now_hpp
