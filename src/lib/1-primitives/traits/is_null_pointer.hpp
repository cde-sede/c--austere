#ifndef ca_primitives_traits_is_null_pointer_hpp
#define ca_primitives_traits_is_null_pointer_hpp

#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS NULL POINTER ----------------------------------------------------

	/* @DOC std::is_null_pointer_v, as a concept. */
	template <typename T>
	concept is_null_pointer = ca::is_same<ca::remove_cv<T>, decltype(nullptr)>;

} // namespace ca

#endif // ca_primitives_traits_is_null_pointer_hpp
