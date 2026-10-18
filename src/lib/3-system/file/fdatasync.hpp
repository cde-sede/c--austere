#ifndef ca_system_file_fdatasync_hpp
#define ca_system_file_fdatasync_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FDATASYNC ------------------------------------------------------

		/* @DOC Flushes the data of fd to the device, and only the metadata needed to read it back. fsync on
		   macOS. Retries on EINTR. */
		[[nodiscard]] auto fdatasync(int fd) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_fdatasync_hpp
