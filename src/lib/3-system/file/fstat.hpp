#ifndef ca_system_file_fstat_hpp
#define ca_system_file_fstat_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/file/file_status.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FSTAT ----------------------------------------------------------

		/* @DOC Status of the file open on fd. */
		[[nodiscard]] auto fstat(int fd) noexcept -> ca::result<ca::sys::file_status>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_fstat_hpp
