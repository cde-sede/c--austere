#ifndef ca_system_file_ftruncate_hpp
#define ca_system_file_ftruncate_hpp

#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FTRUNCATE ------------------------------------------------------

		/* @DOC Sets the size of open file fd: cut, or extended with zeros. Retries on EINTR. */
		[[nodiscard]] auto ftruncate(int fd, ca::u64 size) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_ftruncate_hpp
