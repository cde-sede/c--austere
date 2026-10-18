#ifndef ca_system_file_symlink_hpp
#define ca_system_file_symlink_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SYMLINK --------------------------------------------------------

		/* @DOC Creates a symlink at link pointing to target. */
		[[nodiscard]] auto symlink(const char* target, const char* link) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_symlink_hpp
