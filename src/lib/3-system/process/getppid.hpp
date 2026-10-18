#ifndef ca_system_process_getppid_hpp
#define ca_system_process_getppid_hpp

#include "1-primitives/integers/i32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETPPID --------------------------------------------------------

		/* @DOC Id of the parent process. Never fails. */
		[[nodiscard]] auto getppid(void) noexcept -> ca::i32;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_getppid_hpp
