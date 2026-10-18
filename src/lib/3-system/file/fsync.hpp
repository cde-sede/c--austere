#ifndef ca_system_file_fsync_hpp
#define ca_system_file_fsync_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FSYNC ----------------------------------------------------------

		/* @DOC Flushes fd's data and metadata to the device. */
		[[nodiscard]] auto fsync(int fd) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_fsync_hpp
