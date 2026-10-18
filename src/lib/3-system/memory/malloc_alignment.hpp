#ifndef ca_system_memory_malloc_alignment_hpp
#define ca_system_memory_malloc_alignment_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/max_alignment.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MALLOC ALIGNMENT -----------------------------------------------

		/* @DOC Alignment of every block mallocarray and reallocarray return. */
		inline constexpr ca::usz malloc_alignment = ca::max_alignment;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_malloc_alignment_hpp
