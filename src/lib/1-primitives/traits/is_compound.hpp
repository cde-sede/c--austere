#ifndef ca_primitives_traits_is_compound_hpp
#define ca_primitives_traits_is_compound_hpp

#include "1-primitives/traits/is_fundamental.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS COMPOUND --------------------------------------------------------

	/* @DOC std::is_compound_v, as a concept. */
	template <typename T>
	concept is_compound = !ca::is_fundamental<T>;

} // namespace ca

#endif // ca_primitives_traits_is_compound_hpp
