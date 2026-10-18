#ifndef ca_system_process_kill_hpp
#define ca_system_process_kill_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- KILL -----------------------------------------------------------

		/* @DOC Sends signal (see sys::signals) to process pid; signal 0 only checks that it exists. */
		[[nodiscard]] auto kill(ca::i32 pid, int signal) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_kill_hpp
