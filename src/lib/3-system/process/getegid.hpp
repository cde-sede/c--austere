#ifndef ca_system_process_getegid_hpp
#define ca_system_process_getegid_hpp

#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETEGID --------------------------------------------------------

		/* @DOC Effective group id of the process. Never fails. */
		[[nodiscard]] auto getegid(void) noexcept -> ca::u32;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_getegid_hpp
