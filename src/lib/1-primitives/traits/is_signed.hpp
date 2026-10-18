#ifndef ca_primitives_traits_is_signed_hpp
#define ca_primitives_traits_is_signed_hpp

#include "1-primitives/traits/is_arithmetic.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS SIGNED ----------------------------------------------------------

	/* @DOC std::is_signed_v, as a concept. */
	template <typename T>
	concept is_signed = ca::is_arithmetic<T> && (static_cast<T>(-1) < static_cast<T>(0));

} // namespace ca

#endif // ca_primitives_traits_is_signed_hpp
