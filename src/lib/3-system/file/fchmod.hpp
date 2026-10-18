#ifndef ca_system_file_fchmod_hpp
#define ca_system_file_fchmod_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FCHMOD ---------------------------------------------------------

		/* @DOC Sets the permission bits of open file fd. */
		[[nodiscard]] auto fchmod(int fd, ca::u32 mode) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_fchmod_hpp
