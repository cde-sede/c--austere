#ifndef ca_system_file_dup_hpp
#define ca_system_file_dup_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- DUP ------------------------------------------------------------

		/* @DOC A new descriptor for fd's open file description, close-on-exec when asked
		   (atomically). */
		[[nodiscard]] auto dup(int fd, bool cloexec) noexcept -> ca::result<int>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_dup_hpp
