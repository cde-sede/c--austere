#ifndef ca_system_error_error_message_hpp
#define ca_system_error_error_message_hpp

#include "2-vocabulary/error.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- ERROR MESSAGE --------------------------------------------------

		/* @DOC The system's text for an errno error, untranslated ("No such file or directory");
		   "Unknown error" for codes it does not know. Static storage, thread-safe. */
		[[nodiscard]] auto error_message(ca::error error) noexcept -> ca::sv::str;

	} // namespace sys

} // namespace ca

#endif // ca_system_error_error_message_hpp
