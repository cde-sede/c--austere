#ifndef ca_system_file_write_all_hpp
#define ca_system_file_write_all_hpp

#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- WRITE ALL ------------------------------------------------------

		/* @DOC Writes all of bytes, looping over short writes. On failure an unknown prefix was
		   written. */
		[[nodiscard]] auto write_all(int fd, ca::span<const ca::byte> bytes) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_write_all_hpp
