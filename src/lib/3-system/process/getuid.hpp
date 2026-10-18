#ifndef ca_system_process_getuid_hpp
#define ca_system_process_getuid_hpp

#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETUID ---------------------------------------------------------

		/* @DOC Real user id of the calling process. Never fails. */
		[[nodiscard]] auto getuid(void) noexcept -> ca::u32;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_getuid_hpp
