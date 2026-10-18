#ifndef ca_system_file_fallocate_hpp
#define ca_system_file_fallocate_hpp

#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FALLOCATE ------------------------------------------------------

		/* @DOC Reserves disk space for bytes offset..offset + size of fd, growing the file to cover them;
		   later writes there cannot fail for lack of space. */
		[[nodiscard]] auto fallocate(int fd, ca::u64 offset, ca::u64 size) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_fallocate_hpp
