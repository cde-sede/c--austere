#ifndef ca_system_process_getgid_hpp
#define ca_system_process_getgid_hpp

#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETGID ---------------------------------------------------------

		/* @DOC Real group id of the calling process. Never fails. */
		[[nodiscard]] auto getgid(void) noexcept -> ca::u32;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_getgid_hpp
