#ifndef ca_system_file_truncate_hpp
#define ca_system_file_truncate_hpp

#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TRUNCATE -------------------------------------------------------

		/* @DOC Sets the size of file path: cut, or extended with zeros. */
		[[nodiscard]] auto truncate(const char* path, ca::u64 size) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_truncate_hpp
