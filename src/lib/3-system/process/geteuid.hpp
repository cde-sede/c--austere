#ifndef ca_system_process_geteuid_hpp
#define ca_system_process_geteuid_hpp

#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETEUID --------------------------------------------------------

		/* @DOC Effective user id of the process. Never fails. */
		[[nodiscard]] auto geteuid(void) noexcept -> ca::u32;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_geteuid_hpp
