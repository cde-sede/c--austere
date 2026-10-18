#ifndef ca_primitives_traits_is_array_hpp
#define ca_primitives_traits_is_array_hpp

#include "1-primitives/traits/is_bounded_array.hpp"
#include "1-primitives/traits/is_unbounded_array.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS ARRAY -----------------------------------------------------------

	/* @DOC std::is_array_v, as a concept. */
	template <typename T>
	concept is_array = ca::is_bounded_array<T> || ca::is_unbounded_array<T>;

} // namespace ca

#endif // ca_primitives_traits_is_array_hpp
