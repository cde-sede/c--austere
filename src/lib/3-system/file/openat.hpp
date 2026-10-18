#ifndef ca_system_file_openat_hpp
#define ca_system_file_openat_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- OPENAT ---------------------------------------------------------

		/* @DOC open(2) relative to directory fd dir (or sys::at::cwd) with raw flags; retries on EINTR. */
		[[nodiscard]] auto openat(int dir, const char* path, int flags, ca::u32 mode) noexcept -> ca::result<int>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_openat_hpp
