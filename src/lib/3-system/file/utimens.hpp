#ifndef ca_system_file_utimens_hpp
#define ca_system_file_utimens_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- UTIMENS --------------------------------------------------------

		/* @DOC Sets the access and modification times of path to now. */
		[[nodiscard]] auto utimens(const char* path) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_utimens_hpp
