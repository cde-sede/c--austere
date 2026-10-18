#ifndef ca_system_file_fcntl_hpp
#define ca_system_file_fcntl_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FCNTL ----------------------------------------------------------

		/* @DOC fcntl(2) with no argument (F_GETFD, F_GETFL) or an int one (F_SETFD, F_SETFL). */
		[[nodiscard]] auto fcntl(int fd, int command) noexcept -> ca::result<int>;

		[[nodiscard]] auto fcntl(int fd, int command, int argument) noexcept -> ca::result<int>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_fcntl_hpp
