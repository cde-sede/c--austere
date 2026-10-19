#ifndef ca_system_error_error_name_hpp
#define ca_system_error_error_name_hpp

#include "2-vocabulary/error.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- ERROR NAME -----------------------------------------------------

		/* @DOC The name of error's ca::errc constant ("no_entry"); empty for a code with none.
		   Static storage. */
		[[nodiscard]] auto error_name(ca::error error) noexcept -> ca::sv::str;

	} // namespace sys

} // namespace ca

#endif // ca_system_error_error_name_hpp
