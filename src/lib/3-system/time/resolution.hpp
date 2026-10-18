#ifndef ca_system_time_resolution_hpp
#define ca_system_time_resolution_hpp

#include "1-primitives/integers/i64.hpp"
#include "3-system/time/clock.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- RESOLUTION -----------------------------------------------------

		/* @DOC Resolution of source in nanoseconds: the smallest step now() can report. Never fails. */
		[[nodiscard]] auto resolution(ca::sys::clock source) noexcept -> ca::i64;

	} // namespace sys

} // namespace ca

#endif // ca_system_time_resolution_hpp
