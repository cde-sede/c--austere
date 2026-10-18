#ifndef ca_system_file_chdir_hpp
#define ca_system_file_chdir_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- CHDIR ----------------------------------------------------------

		/* @DOC Makes path the working directory. */
		[[nodiscard]] auto chdir(const char* path) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_chdir_hpp
