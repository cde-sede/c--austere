#ifndef ca_system_process_setsid_hpp
#define ca_system_process_setsid_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SETSID ---------------------------------------------------------

		/* @DOC Makes the process the leader of a new session and process group, without a controlling
		   terminal; returns the session id. Fails in a group leader. */
		[[nodiscard]] auto setsid(void) noexcept -> ca::result<ca::i32>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_setsid_hpp
