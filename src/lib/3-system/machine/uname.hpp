#ifndef ca_system_machine_uname_hpp
#define ca_system_machine_uname_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/machine/system_name.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- UNAME ----------------------------------------------------------

		/* @DOC Names of the running system. */
		[[nodiscard]] auto uname(void) noexcept -> ca::result<ca::sys::system_name>;

	} // namespace sys

} // namespace ca

#endif // ca_system_machine_uname_hpp
