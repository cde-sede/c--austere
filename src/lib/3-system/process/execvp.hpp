#ifndef ca_system_process_execvp_hpp
#define ca_system_process_execvp_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- EXECVP ---------------------------------------------------------

		/* @DOC execve with the current environment, looking file up in $PATH when it has no '/'.
		   Returns only on failure. */
		[[nodiscard]] auto execvp(const char* file, char* const* argv) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_execvp_hpp
