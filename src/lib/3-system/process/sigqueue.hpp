#ifndef ca_system_process_sigqueue_hpp
#define ca_system_process_sigqueue_hpp

#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/i64.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGQUEUE -------------------------------------------------------

		/* @DOC Sends signal to process pid with value, which the receiver's signal_info carries.
		   Real-time signals queue each send; others merge while pending. errc::try_again when the
		   receiver's queue is full; errc::not_supported off Linux. */
		[[nodiscard]] auto sigqueue(ca::i32 pid, int signal, ca::i64 value) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_sigqueue_hpp
