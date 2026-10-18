#ifndef ca_system_file_mkdtemp_hpp
#define ca_system_file_mkdtemp_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MKDTEMP --------------------------------------------------------

		/* @DOC Creates a new directory, mode 0700, from pattern: a path ending in "XXXXXX", rewritten in
		   place to the name chosen. */
		[[nodiscard]] auto mkdtemp(char* pattern) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_mkdtemp_hpp
