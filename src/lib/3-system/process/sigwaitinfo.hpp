#ifndef ca_system_process_sigwaitinfo_hpp
#define ca_system_process_sigwaitinfo_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/process/signal_info.hpp"
#include "3-system/process/signal_set.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGWAITINFO ----------------------------------------------------

		/* @DOC sigwait that also returns who sent the signal and its sigqueue value. errc::not_supported
		   off Linux. */
		[[nodiscard]] auto sigwaitinfo(const ca::sys::signal_set& set) noexcept -> ca::result<ca::sys::signal_info>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_sigwaitinfo_hpp
