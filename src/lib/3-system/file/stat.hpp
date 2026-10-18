#ifndef ca_system_file_stat_hpp
#define ca_system_file_stat_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/file/file_status.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- STAT -----------------------------------------------------------

		/* @DOC Status of path, following symlinks. */
		[[nodiscard]] auto stat(const char* path) noexcept -> ca::result<ca::sys::file_status>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_stat_hpp
