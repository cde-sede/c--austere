#ifndef ca_system_environment_getenv_hpp
#define ca_system_environment_getenv_hpp

#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETENV ---------------------------------------------------------

		/* @DOC Value of environment variable name, or nothing when unset. Valid until the environment
		   changes. */
		[[nodiscard]] auto getenv(const char* name) noexcept -> ca::optional<ca::sv::str>;

	} // namespace sys

} // namespace ca

#endif // ca_system_environment_getenv_hpp
