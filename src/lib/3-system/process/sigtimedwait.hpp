#ifndef ca_system_process_sigtimedwait_hpp
#define ca_system_process_sigtimedwait_hpp

#include "1-primitives/integers/i64.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/process/signal_info.hpp"
#include "3-system/process/signal_set.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGTIMEDWAIT ---------------------------------------------------

		/* @DOC sigwaitinfo for at most nanoseconds: errc::timed_out when no signal of set came by then,
		   at once when nanoseconds is 0 or less and none is pending. Handlers running meanwhile do
		   not extend the wait. errc::not_supported off Linux. */
		[[nodiscard]] auto sigtimedwait(const ca::sys::signal_set& set, ca::i64 nanoseconds) noexcept -> ca::result<ca::sys::signal_info>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_sigtimedwait_hpp
