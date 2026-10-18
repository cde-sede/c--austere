#ifndef ca_system_environment_unsetenv_hpp
#define ca_system_environment_unsetenv_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- UNSETENV -------------------------------------------------------

		/* @DOC Removes environment variable name; a missing one is success. */
		[[nodiscard]] auto unsetenv(const char* name) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_environment_unsetenv_hpp
