#ifndef ca_system_process_try_waitpid_hpp
#define ca_system_process_try_waitpid_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/process/wait_status.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TRY WAITPID ----------------------------------------------------

		/* @DOC waitpid without blocking: nothing while child pid still runs. */
		[[nodiscard]] auto try_waitpid(ca::i32 pid) noexcept -> ca::result<ca::optional<ca::sys::wait_status>>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_try_waitpid_hpp
