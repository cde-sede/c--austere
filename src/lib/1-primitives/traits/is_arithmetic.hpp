#ifndef ca_primitives_traits_is_arithmetic_hpp
#define ca_primitives_traits_is_arithmetic_hpp

#include "1-primitives/traits/is_floating_point.hpp"
#include "1-primitives/traits/is_integral.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS ARITHMETIC ------------------------------------------------------

	/* @DOC std::is_arithmetic_v, as a concept. */
	template <typename T>
	concept is_arithmetic = ca::is_integral<T> || ca::is_floating_point<T>;

} // namespace ca

#endif // ca_primitives_traits_is_arithmetic_hpp
