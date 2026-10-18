#ifndef ca_primitives_traits_bool_constant_hpp
#define ca_primitives_traits_bool_constant_hpp

#include "1-primitives/traits/integral_constant.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BOOL CONSTANT ------------------------------------------------------

	/* @DOC std::bool_constant. */
	template <bool B>
	using bool_constant = ca::integral_constant<bool, B>;

} // namespace ca

#endif // ca_primitives_traits_bool_constant_hpp
