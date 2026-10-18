#ifndef ca_primitives_traits_is_unsigned_hpp
#define ca_primitives_traits_is_unsigned_hpp

#include "1-primitives/traits/is_arithmetic.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS UNSIGNED --------------------------------------------------------

	/* @DOC std::is_unsigned_v, as a concept. */
	template <typename T>
	concept is_unsigned = ca::is_arithmetic<T> && (static_cast<T>(0) < static_cast<T>(-1));

} // namespace ca

#endif // ca_primitives_traits_is_unsigned_hpp
