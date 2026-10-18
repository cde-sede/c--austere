#ifndef ca_system_file_opendir_hpp
#define ca_system_file_opendir_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/file/directory_stream.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- OPENDIR --------------------------------------------------------

		/* @DOC Opens directory path for listing; close it with closedir. */
		[[nodiscard]] auto opendir(const char* path) noexcept -> ca::result<ca::sys::directory_stream*>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_opendir_hpp
