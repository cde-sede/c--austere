#ifndef ca_system_process_execve_hpp
#define ca_system_process_execve_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- EXECVE ---------------------------------------------------------

		/* @DOC Replaces the process image with program path, arguments argv and environment envp
		   (both null-terminated). Returns only on failure. */
		[[nodiscard]] auto execve(const char* path, char* const* argv, char* const* envp) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_execve_hpp
