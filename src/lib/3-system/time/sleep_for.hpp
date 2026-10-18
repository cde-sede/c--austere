#ifndef ca_system_time_sleep_for_hpp
#define ca_system_time_sleep_for_hpp

#include "1-primitives/integers/i64.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SLEEP FOR ------------------------------------------------------

		/* @DOC Suspends the thread for nanoseconds, resuming after signals until the time is up. */
		[[nodiscard]] auto sleep_for(ca::i64 nanoseconds) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_time_sleep_for_hpp
