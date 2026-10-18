#ifndef ca_system_file_unlink_hpp
#define ca_system_file_unlink_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- UNLINK ---------------------------------------------------------

		/* @DOC Removes the directory entry path (not a directory). */
		[[nodiscard]] auto unlink(const char* path) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_unlink_hpp
