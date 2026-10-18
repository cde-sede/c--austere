#ifndef ca_system_file_fdopendir_hpp
#define ca_system_file_fdopendir_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/file/directory_stream.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FDOPENDIR ------------------------------------------------------

		/* @DOC Lists open directory fd, which the stream then owns: closedir closes it. */
		[[nodiscard]] auto fdopendir(int fd) noexcept -> ca::result<ca::sys::directory_stream*>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_fdopendir_hpp
