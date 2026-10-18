#ifndef ca_system_file_chown_hpp
#define ca_system_file_chown_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/file/unchanged_id.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- CHOWN ----------------------------------------------------------

		/* @DOC Sets the owner of path, following symlinks; sys::unchanged_id keeps user or group. */
		[[nodiscard]] auto chown(const char* path, ca::u32 user, ca::u32 group) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_chown_hpp
