#ifndef ca_system_file_renameat_hpp
#define ca_system_file_renameat_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- RENAMEAT -------------------------------------------------------

		/* @DOC Renames from (relative to from_dir) to to (relative to to_dir), replacing it. */
		[[nodiscard]] auto renameat(int from_dir, const char* from, int to_dir, const char* to) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_renameat_hpp
