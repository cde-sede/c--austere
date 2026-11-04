#ifndef ca_system_syscall_impl_read_own_hpp
#define ca_system_syscall_impl_read_own_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYSCALL ------------------------------------------------------------

	namespace syscall {


		// -- READ OWN -------------------------------------------------------

		namespace impl {

			/* @DOC Copies out.size() bytes of the process's own memory at address into out through
			   the kernel (process_vm_readv), which fails instead of faulting on unmapped memory:
			   whether they were all read. Raw, so the last error stays. Linux x86-64; false
			   elsewhere. */
			[[nodiscard]] auto read_own(ca::usz address, ca::span<ca::byte> out) noexcept -> bool;

		} // namespace impl

	} // namespace syscall

} // namespace ca

#endif // ca_system_syscall_impl_read_own_hpp
