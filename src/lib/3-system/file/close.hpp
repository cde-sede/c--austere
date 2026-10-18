#ifndef ca_system_file_close_hpp
#define ca_system_file_close_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- CLOSE ----------------------------------------------------------

		/* @DOC Closes fd. The descriptor is gone even on failure; EINTR counts as success (retrying
		   could close a reused descriptor). */
		[[nodiscard]] auto close(int fd) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_close_hpp
