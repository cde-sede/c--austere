#ifndef ca_primitives_memory_max_alignment_hpp
#define ca_primitives_memory_max_alignment_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MAX ALIGNMENT ------------------------------------------------------

	/* @DOC alignof(max_align_t), from the compiler: the strictest alignment of a scalar type, and
	   what operator new and malloc guarantee. Checked against <stddef.h> when the library
	   builds. */
	inline constexpr ca::usz max_alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__;

} // namespace ca

#endif // ca_primitives_memory_max_alignment_hpp
