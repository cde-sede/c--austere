#ifndef ca_system_file_readdir_hpp
#define ca_system_file_readdir_hpp

#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/file/directory_stream.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- READDIR --------------------------------------------------------

		/* @DOC Next entry name, "." and ".." skipped; empty at the end. Valid until the next call
		   on stream. */
		[[nodiscard]] auto readdir(ca::sys::directory_stream* stream) noexcept -> ca::result<ca::sv::str>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_readdir_hpp
