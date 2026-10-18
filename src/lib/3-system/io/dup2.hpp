#ifndef ca_system_io_dup2_hpp
#define ca_system_io_dup2_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- DUP2 -----------------------------------------------------------

		/* @DOC Makes target a copy of fd, closing what target held; the copy is not close-on-exec. */
		[[nodiscard]] auto dup2(int fd, int target) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_dup2_hpp
