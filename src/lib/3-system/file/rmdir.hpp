#ifndef ca_system_file_rmdir_hpp
#define ca_system_file_rmdir_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- RMDIR ----------------------------------------------------------

		/* @DOC Removes empty directory path. */
		[[nodiscard]] auto rmdir(const char* path) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_rmdir_hpp
