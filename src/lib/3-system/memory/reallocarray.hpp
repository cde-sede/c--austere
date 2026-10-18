#ifndef ca_system_memory_reallocarray_hpp
#define ca_system_memory_reallocarray_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- REALLOCARRAY ---------------------------------------------------

		/* @DOC Resizes block (nullptr: allocates one) to count * size bytes, aligned to
		   malloc_alignment; contents kept up to the smaller size. Zero bytes counts as one. On failure
		   block is untouched; ENOMEM also when count * size overflows. */
		[[nodiscard]] auto reallocarray(void* block, ca::usz count, ca::usz size) noexcept -> ca::result<void*>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_reallocarray_hpp
