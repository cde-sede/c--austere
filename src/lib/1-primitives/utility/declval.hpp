#ifndef ca_primitives_utility_declval_hpp
#define ca_primitives_utility_declval_hpp

#include "1-primitives/traits/add_rvalue_reference.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DECLVAL ------------------------------------------------------------

	/* @DOC Unevaluated value of type T&& (void for void). Never called. */
	template <typename T>
	auto declval(void) noexcept -> ca::add_rvalue_reference<T>;

} // namespace ca

#endif // ca_primitives_utility_declval_hpp
