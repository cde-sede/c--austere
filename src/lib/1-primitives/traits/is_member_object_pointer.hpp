#ifndef ca_primitives_traits_is_member_object_pointer_hpp
#define ca_primitives_traits_is_member_object_pointer_hpp

#include "1-primitives/traits/is_member_function_pointer.hpp"
#include "1-primitives/traits/is_member_pointer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS MEMBER OBJECT POINTER -------------------------------------------

	/* @DOC std::is_member_object_pointer_v, as a concept. */
	template <typename T>
	concept is_member_object_pointer = ca::is_member_pointer<T> && !ca::is_member_function_pointer<T>;

} // namespace ca

#endif // ca_primitives_traits_is_member_object_pointer_hpp
