#ifndef ca_system_process_waitpid_hpp
#define ca_system_process_waitpid_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/process/wait_status.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- WAITPID --------------------------------------------------------

		/* @DOC Waits for child pid to end and reaps it. Retries on EINTR. */
		[[nodiscard]] auto waitpid(ca::i32 pid) noexcept -> ca::result<ca::sys::wait_status>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_waitpid_hpp
