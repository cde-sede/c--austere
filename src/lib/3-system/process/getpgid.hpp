#ifndef ca_system_process_getpgid_hpp
#define ca_system_process_getpgid_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETPGID --------------------------------------------------------

		/* @DOC Process group of process pid (0: the caller). */
		[[nodiscard]] auto getpgid(ca::i32 pid) noexcept -> ca::result<ca::i32>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_getpgid_hpp
