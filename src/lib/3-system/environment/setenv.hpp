#ifndef ca_system_environment_setenv_hpp
#define ca_system_environment_setenv_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SETENV ---------------------------------------------------------

		/* @DOC Sets environment variable name to value; an existing one is kept unless overwrite. Not
		   thread-safe against concurrent getenv. */
		[[nodiscard]] auto setenv(const char* name, const char* value, bool overwrite) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_environment_setenv_hpp
