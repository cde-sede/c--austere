#ifndef ca_system_process_setpgid_hpp
#define ca_system_process_setpgid_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SETPGID --------------------------------------------------------

		/* @DOC Moves process pid (0: the caller) into process group group (0: a new one led by pid). */
		[[nodiscard]] auto setpgid(ca::i32 pid, ca::i32 group) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_setpgid_hpp
