#ifndef ca_system_file_rename_noreplace_hpp
#define ca_system_file_rename_noreplace_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- RENAME NOREPLACE -----------------------------------------------

		/* @DOC Renames from to to, failing with errc::exists when to exists, atomically. errc::invalid_argument
		   or errc::not_supported where the file system cannot. */
		[[nodiscard]] auto rename_noreplace(const char* from, const char* to) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_rename_noreplace_hpp
