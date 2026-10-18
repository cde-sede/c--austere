#ifndef ca_system_memory_msync_hpp
#define ca_system_memory_msync_hpp

#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MSYNC ----------------------------------------------------------

		/* @DOC Writes the changed pages of shared file mapping (page-aligned) back to the file and
		   waits. */
		[[nodiscard]] auto msync(ca::span<ca::byte> mapping) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_msync_hpp
