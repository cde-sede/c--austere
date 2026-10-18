#ifndef ca_system_file_mkdirat_hpp
#define ca_system_file_mkdirat_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MKDIRAT --------------------------------------------------------

		/* @DOC Creates directory path relative to directory dir with mode (umask applies). */
		[[nodiscard]] auto mkdirat(int dir, const char* path, ca::u32 mode) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_mkdirat_hpp
