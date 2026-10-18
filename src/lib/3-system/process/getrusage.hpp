#ifndef ca_system_process_getrusage_hpp
#define ca_system_process_getrusage_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/process/resource_usage.hpp"
#include "3-system/process/usage_target.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETRUSAGE ------------------------------------------------------

		/* @DOC Resources used by target so far. */
		[[nodiscard]] auto getrusage(ca::sys::usage_target target) noexcept -> ca::result<ca::sys::resource_usage>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_getrusage_hpp
