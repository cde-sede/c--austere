#ifndef ca_primitives_traits_is_nothrow_default_constructible_hpp
#define ca_primitives_traits_is_nothrow_default_constructible_hpp

#include "1-primitives/traits/is_nothrow_constructible.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS NOTHROW DEFAULT CONSTRUCTIBLE -----------------------------------

	/* @DOC std::is_nothrow_default_constructible_v, as a concept. */
	template <typename T>
	concept is_nothrow_default_constructible = ca::is_nothrow_constructible<T>;

} // namespace ca

#endif // ca_primitives_traits_is_nothrow_default_constructible_hpp
