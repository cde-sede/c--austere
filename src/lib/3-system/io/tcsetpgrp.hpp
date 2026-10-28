#ifndef ca_system_io_tcsetpgrp_hpp
#define ca_system_io_tcsetpgrp_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TCSETPGRP ------------------------------------------------------

		/* @DOC Makes group the foreground process group of the terminal fd is attached to, the
		   caller's controlling terminal, group being in the caller's session. Called from a
		   background group, it stops the caller with signals::background_write unless that signal
		   is ignored or blocked, as a shell does while it hands the terminal back and forth. */
		[[nodiscard]] auto tcsetpgrp(int fd, ca::i32 group) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_tcsetpgrp_hpp
