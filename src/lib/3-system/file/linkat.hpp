#ifndef ca_system_file_linkat_hpp
#define ca_system_file_linkat_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- LINKAT ---------------------------------------------------------

		/* @DOC Hard link to (relative to to_dir) to from (relative to from_dir); flags: 0, or
		   sys::at::symlink_follow to link the target of a symlink from. */
		[[nodiscard]] auto linkat(int from_dir, const char* from, int to_dir, const char* to, int flags) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_linkat_hpp
