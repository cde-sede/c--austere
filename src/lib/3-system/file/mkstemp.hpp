#ifndef ca_system_file_mkstemp_hpp
#define ca_system_file_mkstemp_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MKSTEMP --------------------------------------------------------

		/* @DOC Creates and opens a new file, read-write, mode 0600, from pattern: a path ending in
		   "XXXXXX", rewritten in place to the name chosen. flags: 0, or the OPEN_VALUE of fs::cloexec,
		   fs::append and fs::sync. */
		[[nodiscard]] auto mkstemp(char* pattern, int flags) noexcept -> ca::result<int>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_mkstemp_hpp
