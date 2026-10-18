#ifndef ca_primitives_traits_is_nothrow_copy_constructible_hpp
#define ca_primitives_traits_is_nothrow_copy_constructible_hpp

#include "1-primitives/traits/add_lvalue_reference.hpp"
#include "1-primitives/traits/is_nothrow_constructible.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS NOTHROW COPY CONSTRUCTIBLE --------------------------------------

	/* @DOC std::is_nothrow_copy_constructible_v, as a concept. */
	template <typename T>
	concept is_nothrow_copy_constructible = ca::is_nothrow_constructible<T, ca::add_lvalue_reference<const T>>;

} // namespace ca

#endif // ca_primitives_traits_is_nothrow_copy_constructible_hpp
