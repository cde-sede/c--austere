#ifndef ca_primitives_traits_is_void_hpp
#define ca_primitives_traits_is_void_hpp

#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS VOID ------------------------------------------------------------

	/* @DOC std::is_void_v, as a concept. */
	template <typename T>
	concept is_void = ca::is_same<ca::remove_cv<T>, void>;

} // namespace ca

#endif // ca_primitives_traits_is_void_hpp
