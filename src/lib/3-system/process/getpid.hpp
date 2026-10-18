#ifndef ca_system_process_getpid_hpp
#define ca_system_process_getpid_hpp

#include "1-primitives/integers/i32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETPID ---------------------------------------------------------

		/* @DOC Id of the calling process. Never fails. */
		[[nodiscard]] auto getpid(void) noexcept -> ca::i32;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_getpid_hpp
