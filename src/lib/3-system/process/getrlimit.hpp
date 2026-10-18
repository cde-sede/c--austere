#ifndef ca_system_process_getrlimit_hpp
#define ca_system_process_getrlimit_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/process/resource.hpp"
#include "3-system/process/resource_limit.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETRLIMIT ------------------------------------------------------

		/* @DOC Current and maximum bound of which. */
		[[nodiscard]] auto getrlimit(ca::sys::resource which) noexcept -> ca::result<ca::sys::resource_limit>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_getrlimit_hpp
