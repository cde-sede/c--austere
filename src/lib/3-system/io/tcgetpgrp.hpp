#ifndef ca_system_io_tcgetpgrp_hpp
#define ca_system_io_tcgetpgrp_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TCGETPGRP ------------------------------------------------------

		/* @DOC Foreground process group of the terminal fd is attached to, which must be the calling
		   process's controlling terminal; not_a_terminal otherwise. */
		[[nodiscard]] auto tcgetpgrp(int fd) noexcept -> ca::result<ca::i32>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_tcgetpgrp_hpp
