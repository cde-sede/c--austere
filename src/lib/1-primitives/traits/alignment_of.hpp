#ifndef ca_primitives_traits_alignment_of_hpp
#define ca_primitives_traits_alignment_of_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ALIGNMENT OF -------------------------------------------------------

	/* @DOC std::alignment_of_v. */
	template <typename T>
	inline constexpr ca::usz alignment_of = alignof(T);

} // namespace ca

#endif // ca_primitives_traits_alignment_of_hpp
