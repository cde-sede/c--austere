#ifndef ca_system_file_sync_hpp
#define ca_system_file_sync_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SYNC -----------------------------------------------------------

		/* @DOC () causes all pending modifications to filesystem metadata and
		   cached file data to be written to the underlying filesystems.
		   The operation does not report synchronization errors. */
		[[nodiscard]] auto sync(void) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_sync_hpp
