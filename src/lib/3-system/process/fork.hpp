#ifndef ca_system_process_fork_hpp
#define ca_system_process_fork_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FORK -----------------------------------------------------------

		/* @DOC Duplicates the calling process: the child's id in the parent, 0 in the child. In a
		   multithreaded parent the child may only make async-signal-safe calls before exec. */
		[[nodiscard]] auto fork(void) noexcept -> ca::result<ca::i32>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_fork_hpp
