#ifndef ca_system_file_open_hpp
#define ca_system_file_open_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- OPEN -----------------------------------------------------------

		/* @DOC open(2) with raw flags; retries on EINTR. Prefer ca::fs::descriptor::open. */
		[[nodiscard]] auto open(const char* path, int flags, ca::u32 mode) noexcept -> ca::result<int>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_open_hpp
