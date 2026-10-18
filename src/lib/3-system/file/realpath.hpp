#ifndef ca_system_file_realpath_hpp
#define ca_system_file_realpath_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- REALPATH -------------------------------------------------------

		/* @DOC Absolute path with no symlink, "." or ".." component, in a block to release with
		   ca::sys::free. Every component must exist. */
		[[nodiscard]] auto realpath(const char* path) noexcept -> ca::result<char*>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_realpath_hpp
