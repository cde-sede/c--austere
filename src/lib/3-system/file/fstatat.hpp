#ifndef ca_system_file_fstatat_hpp
#define ca_system_file_fstatat_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/file/file_status.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FSTATAT --------------------------------------------------------

		/* @DOC Status of path relative to directory dir; flags: 0, or sys::at::symlink_nofollow. */
		[[nodiscard]] auto fstatat(int dir, const char* path, int flags) noexcept -> ca::result<ca::sys::file_status>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_fstatat_hpp
