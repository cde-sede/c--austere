#ifndef ca_primitives_traits_is_swappable_hpp
#define ca_primitives_traits_is_swappable_hpp

#include "1-primitives/traits/add_lvalue_reference.hpp"
#include "1-primitives/traits/is_swappable_with.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS SWAPPABLE -------------------------------------------------------

	/* @DOC std::is_swappable_v, as a concept. */
	template <typename T>
	concept is_swappable = ca::is_swappable_with<ca::add_lvalue_reference<T>, ca::add_lvalue_reference<T>>;

} // namespace ca

#endif // ca_primitives_traits_is_swappable_hpp
