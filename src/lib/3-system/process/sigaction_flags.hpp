#ifndef ca_system_process_sigaction_flags_hpp
#define ca_system_process_sigaction_flags_hpp

#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGACTION FLAGS ------------------------------------------------

		/* @DOC The flags of sigaction, SA_* without <signal.h>. Checked against it when the library
		   builds. restart is SA_RESTART, on_alternate_stack SA_ONSTACK (the handler runs on the
		   sigaltstack stack), no_defer SA_NODEFER, reset_handler SA_RESETHAND (one delivery, then
		   the default action). */
		namespace sigaction_flags {

#if ca_platform(linux)
			inline constexpr ca::u64 on_alternate_stack = 0x08000000U;
			inline constexpr ca::u64 restart            = 0x10000000U;
			inline constexpr ca::u64 no_defer           = 0x40000000U;
			inline constexpr ca::u64 reset_handler      = 0x80000000U;
#else
			inline constexpr ca::u64 on_alternate_stack = 0x1U;
			inline constexpr ca::u64 restart            = 0x2U;
			inline constexpr ca::u64 reset_handler      = 0x4U;
			inline constexpr ca::u64 no_defer           = 0x10U;
#endif

		} // namespace sigaction_flags

	} // namespace sys

} // namespace ca

#endif // ca_system_process_sigaction_flags_hpp
