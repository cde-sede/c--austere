#ifndef ca_system_file_faccessat_hpp
#define ca_system_file_faccessat_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FACCESSAT ------------------------------------------------------

		/* @DOC access relative to directory dir; flags: 0, or any of sys::at::effective_ids and
		   sys::at::symlink_nofollow. */
		[[nodiscard]] auto faccessat(int dir, const char* path, int checks, int flags) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_faccessat_hpp
