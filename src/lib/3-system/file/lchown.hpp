#ifndef ca_system_file_lchown_hpp
#define ca_system_file_lchown_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/file/unchanged_id.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- LCHOWN ---------------------------------------------------------

		/* @DOC Sets the owner of path itself, not of a symlink target; sys::unchanged_id keeps user or group. */
		[[nodiscard]] auto lchown(const char* path, ca::u32 user, ca::u32 group) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_lchown_hpp
