#ifndef ca_system_memory_mallocarray_hpp
#define ca_system_memory_mallocarray_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MALLOCARRAY ----------------------------------------------------

		/* @DOC Allocates an uninitialized block of count * size bytes, aligned to malloc_alignment;
		   release it with free. Zero bytes counts as one. ENOMEM also when count * size overflows. */
		[[nodiscard]] auto mallocarray(ca::usz count, ca::usz size) noexcept -> ca::result<void*>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_mallocarray_hpp
