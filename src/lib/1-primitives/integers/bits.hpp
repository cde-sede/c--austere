#ifndef ca_primitives_integers_bits_hpp
#define ca_primitives_integers_bits_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BITS ---------------------------------------------------------------

	/* @DOC Bits in the object representation of T. */
	template <typename T>
	inline constexpr ca::usz bits = sizeof(T) * __CHAR_BIT__;

} // namespace ca

#endif // ca_primitives_integers_bits_hpp
