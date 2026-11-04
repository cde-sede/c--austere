#ifndef ca_system_memory_mmap_anonymous_fixed_hpp
#define ca_system_memory_mmap_anonymous_fixed_hpp

#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MMAP ANONYMOUS FIXED -------------------------------------------

		/* @DOC Maps fresh zeroed pages over region, page-aligned, replacing the pages there
		   (MAP_FIXED), private, with protection (sys::protection bits). region must lie in
		   memory the caller owns. */
		[[nodiscard]] auto mmap_anonymous_fixed(ca::span<ca::byte> region, int protection) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_mmap_anonymous_fixed_hpp
