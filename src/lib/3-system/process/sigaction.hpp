#ifndef ca_system_process_sigaction_hpp
#define ca_system_process_sigaction_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/process/signal_disposition.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGACTION ------------------------------------------------------

		/* @DOC Runs handler on each delivery of signal, with interrupted calls restarted (SA_RESTART).
		   The handler may only make async-signal-safe calls. */
		[[nodiscard]] auto sigaction(int signal, void (*handler)(int)) noexcept -> ca::result<void>;

		/* @DOC Makes signal take its default action, or be ignored. */
		[[nodiscard]] auto sigaction(int signal, ca::sys::signal_disposition disposition) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_sigaction_hpp
