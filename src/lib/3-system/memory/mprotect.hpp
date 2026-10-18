#ifndef ca_system_memory_mprotect_hpp
#define ca_system_memory_mprotect_hpp

#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MPROTECT -------------------------------------------------------

		/* @DOC Sets the protection (sys::protection bits) of the pages of region, page-aligned. */
		[[nodiscard]] auto mprotect(ca::span<ca::byte> region, int protection) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_mprotect_hpp
