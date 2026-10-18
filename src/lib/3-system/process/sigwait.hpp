#ifndef ca_system_process_sigwait_hpp
#define ca_system_process_sigwait_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/process/signal_set.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGWAIT --------------------------------------------------------

		/* @DOC Waits for one signal of set to be pending, takes it, and returns its number. Block set first
		   (sigprocmask), in every thread. */
		[[nodiscard]] auto sigwait(const ca::sys::signal_set& set) noexcept -> ca::result<int>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_sigwait_hpp
