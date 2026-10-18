#ifndef ca_system_file_closedir_hpp
#define ca_system_file_closedir_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/file/directory_stream.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- CLOSEDIR -------------------------------------------------------

		/* @DOC Closes stream; it is gone even on failure. */
		[[nodiscard]] auto closedir(ca::sys::directory_stream* stream) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_closedir_hpp
