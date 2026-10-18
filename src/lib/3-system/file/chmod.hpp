#ifndef ca_system_file_chmod_hpp
#define ca_system_file_chmod_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- CHMOD ----------------------------------------------------------

		/* @DOC Sets the permission bits of path. */
		[[nodiscard]] auto chmod(const char* path, ca::u32 mode) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_chmod_hpp
