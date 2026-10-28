#ifndef ca_system_process_sigaltstack_hpp
#define ca_system_process_sigaltstack_hpp

#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGALTSTACK ----------------------------------------------------

		/* @DOC Makes stack the calling thread's alternate signal stack, where handlers installed with
		   sigaction_flags::on_alternate_stack run; an empty stack removes it. Too small a stack fails
		   with errc::out_of_memory (the kernel minimum depends on the CPU's register state; 64 KiB is
		   ample), a change while a handler runs on it with errc::not_permitted. stack must outlive
		   its use: remove it before freeing the memory or ending the thread. */
		[[nodiscard]] auto sigaltstack(ca::span<char> stack) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_sigaltstack_hpp
