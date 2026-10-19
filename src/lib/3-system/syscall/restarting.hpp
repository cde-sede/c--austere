#ifndef ca_system_syscall_restarting_hpp
#define ca_system_syscall_restarting_hpp

#include "3-system/error/errc.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYSCALL ------------------------------------------------------------

	namespace syscall {


		// -- RESTARTING -----------------------------------------------------

		/* @DOC call() again while it fails with errc::interrupted (a signal handler ran before the
		   syscall did anything); its first other result. */
		template <typename F>
		[[nodiscard]] inline auto restarting(const F& call) noexcept -> decltype(call()) {
			for (;;) {
				auto done = call();
				if (done || done.error() != ca::errc::interrupted)
					return done;
			}
		}

	} // namespace syscall

} // namespace ca

#endif // ca_system_syscall_restarting_hpp
