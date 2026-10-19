#ifndef ca_system_syscall_auxiliary_hpp
#define ca_system_syscall_auxiliary_hpp

#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYSCALL ------------------------------------------------------------

	namespace syscall {


		// -- AUXILIARY ------------------------------------------------------

		/* @DOC Value of entry type (AT_*) of the auxiliary vector the kernel handed the process, 0
		   when absent. Read from /proc/self/auxv at each call: for one-time lookups (the vDSO). Linux
		   x86-64; 0 elsewhere. */
		[[nodiscard]] auto auxiliary(ca::u64 type) noexcept -> ca::u64;

	} // namespace syscall

} // namespace ca

#endif // ca_system_syscall_auxiliary_hpp
