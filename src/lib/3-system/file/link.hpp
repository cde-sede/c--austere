#ifndef ca_system_file_link_hpp
#define ca_system_file_link_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- LINK -----------------------------------------------------------

		/* @DOC Hard link to pointing at the same file as from. */
		[[nodiscard]] auto link(const char* from, const char* to) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_link_hpp
