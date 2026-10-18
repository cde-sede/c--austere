#ifndef ca_primitives_traits_is_nothrow_move_constructible_hpp
#define ca_primitives_traits_is_nothrow_move_constructible_hpp

#include "1-primitives/traits/add_rvalue_reference.hpp"
#include "1-primitives/traits/is_nothrow_constructible.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS NOTHROW MOVE CONSTRUCTIBLE --------------------------------------

	/* @DOC std::is_nothrow_move_constructible_v, as a concept. */
	template <typename T>
	concept is_nothrow_move_constructible = ca::is_nothrow_constructible<T, ca::add_rvalue_reference<T>>;

} // namespace ca

#endif // ca_primitives_traits_is_nothrow_move_constructible_hpp
