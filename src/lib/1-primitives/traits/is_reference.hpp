#ifndef ca_primitives_traits_is_reference_hpp
#define ca_primitives_traits_is_reference_hpp

#include "1-primitives/traits/is_lvalue_reference.hpp"
#include "1-primitives/traits/is_rvalue_reference.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS REFERENCE -------------------------------------------------------

	/* @DOC std::is_reference_v, as a concept. */
	template <typename T>
	concept is_reference = ca::is_lvalue_reference<T> || ca::is_rvalue_reference<T>;

} // namespace ca

#endif // ca_primitives_traits_is_reference_hpp
