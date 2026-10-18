#ifndef ca_primitives_traits_is_fundamental_hpp
#define ca_primitives_traits_is_fundamental_hpp

#include "1-primitives/traits/is_arithmetic.hpp"
#include "1-primitives/traits/is_null_pointer.hpp"
#include "1-primitives/traits/is_void.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS FUNDAMENTAL -----------------------------------------------------

	/* @DOC std::is_fundamental_v, as a concept. */
	template <typename T>
	concept is_fundamental = ca::is_arithmetic<T> || ca::is_void<T> || ca::is_null_pointer<T>;

} // namespace ca

#endif // ca_primitives_traits_is_fundamental_hpp
