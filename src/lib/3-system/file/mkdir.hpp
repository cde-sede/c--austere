#ifndef ca_system_file_mkdir_hpp
#define ca_system_file_mkdir_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MKDIR ----------------------------------------------------------

		/* @DOC Creates directory path with mode (umask applies). */
		[[nodiscard]] auto mkdir(const char* path, ca::u32 mode) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_mkdir_hpp
