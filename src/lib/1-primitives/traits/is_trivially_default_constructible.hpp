#ifndef ca_primitives_traits_is_trivially_default_constructible_hpp
#define ca_primitives_traits_is_trivially_default_constructible_hpp

#include "1-primitives/traits/is_trivially_constructible.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS TRIVIALLY DEFAULT CONSTRUCTIBLE ---------------------------------

	/* @DOC std::is_trivially_default_constructible_v, as a concept. */
	template <typename T>
	concept is_trivially_default_constructible = ca::is_trivially_constructible<T>;

} // namespace ca

#endif // ca_primitives_traits_is_trivially_default_constructible_hpp
