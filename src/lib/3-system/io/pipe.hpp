#ifndef ca_system_io_pipe_hpp
#define ca_system_io_pipe_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/io/pipe_ends.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- PIPE -----------------------------------------------------------

		/* @DOC A new pipe. flags: 0, or the OPEN_VALUE of fs::cloexec and fs::nonblock, applied to
		   both ends (atomically where the platform has pipe2). */
		[[nodiscard]] auto pipe(int flags) noexcept -> ca::result<ca::sys::pipe_ends>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_pipe_hpp
