#ifndef ca_system_file_dirfd_hpp
#define ca_system_file_dirfd_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/file/directory_stream.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- DIRFD ----------------------------------------------------------

		/* @DOC Descriptor of the directory stream lists, for the *at calls. The stream still owns it. */
		[[nodiscard]] auto dirfd(ca::sys::directory_stream* stream) noexcept -> ca::result<int>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_dirfd_hpp
