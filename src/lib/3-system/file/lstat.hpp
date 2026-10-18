#ifndef ca_system_file_lstat_hpp
#define ca_system_file_lstat_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/file/file_status.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- LSTAT ----------------------------------------------------------

		/* @DOC Status of path; a symlink describes itself. */
		[[nodiscard]] auto lstat(const char* path) noexcept -> ca::result<ca::sys::file_status>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_lstat_hpp
