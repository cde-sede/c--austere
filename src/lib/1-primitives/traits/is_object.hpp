#ifndef ca_primitives_traits_is_object_hpp
#define ca_primitives_traits_is_object_hpp

#include "1-primitives/traits/is_function.hpp"
#include "1-primitives/traits/is_reference.hpp"
#include "1-primitives/traits/is_void.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS OBJECT ----------------------------------------------------------

	/* @DOC std::is_object_v, as a concept. */
	template <typename T>
	concept is_object = !ca::is_function<T> && !ca::is_reference<T> && !ca::is_void<T>;

} // namespace ca

#endif // ca_primitives_traits_is_object_hpp
