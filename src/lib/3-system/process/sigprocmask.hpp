#ifndef ca_system_process_sigprocmask_hpp
#define ca_system_process_sigprocmask_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/process/mask_change.hpp"
#include "3-system/process/signal_set.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGPROCMASK ----------------------------------------------------

		/* @DOC Changes the signals the calling thread blocks; returns those it blocked before. */
		[[nodiscard]] auto sigprocmask(ca::sys::mask_change how, const ca::sys::signal_set& set) noexcept -> ca::result<ca::sys::signal_set>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_sigprocmask_hpp
