#ifndef ca_system_file_rename_hpp
#define ca_system_file_rename_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- RENAME ---------------------------------------------------------

		/* @DOC Renames from to to, replacing a file at to. */
		[[nodiscard]] auto rename(const char* from, const char* to) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_rename_hpp
