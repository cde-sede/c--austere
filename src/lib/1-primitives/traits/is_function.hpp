#ifndef ca_primitives_traits_is_function_hpp
#define ca_primitives_traits_is_function_hpp

#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_reference.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS FUNCTION --------------------------------------------------------

	/* @DOC std::is_function_v, as a concept. */
	template <typename T>
	concept is_function = !ca::is_const<const T> && !ca::is_reference<T>;

} // namespace ca

#endif // ca_primitives_traits_is_function_hpp
