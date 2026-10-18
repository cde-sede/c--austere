#ifndef ca_system_file_unlinkat_hpp
#define ca_system_file_unlinkat_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- UNLINKAT -------------------------------------------------------

		/* @DOC Removes path relative to directory dir; flags: 0 for a file, sys::at::remove_directory for an
		   empty directory. */
		[[nodiscard]] auto unlinkat(int dir, const char* path, int flags) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_unlinkat_hpp
