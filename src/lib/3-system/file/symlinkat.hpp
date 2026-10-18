#ifndef ca_system_file_symlinkat_hpp
#define ca_system_file_symlinkat_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SYMLINKAT ------------------------------------------------------

		/* @DOC Creates symlink link (relative to directory dir) pointing at target, stored as given. */
		[[nodiscard]] auto symlinkat(const char* target, int dir, const char* link) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_symlinkat_hpp
