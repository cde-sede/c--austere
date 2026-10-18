#ifndef ca_system_file_access_hpp
#define ca_system_file_access_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- ACCESS ---------------------------------------------------------

		/* @DOC Succeeds when the real user may access path as checks (sys::access_checks) say;
		   errc::access_denied or errc::no_entry otherwise. */
		[[nodiscard]] auto access(const char* path, int checks) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_access_hpp
