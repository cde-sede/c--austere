#ifndef ca_system_process_setrlimit_hpp
#define ca_system_process_setrlimit_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/process/resource.hpp"
#include "3-system/process/resource_limit.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SETRLIMIT ------------------------------------------------------

		/* @DOC Sets the current and maximum bound of which. */
		[[nodiscard]] auto setrlimit(ca::sys::resource which, const ca::sys::resource_limit& limit) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_setrlimit_hpp
