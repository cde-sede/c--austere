#ifndef ca_primitives_traits_is_member_function_pointer_hpp
#define ca_primitives_traits_is_member_function_pointer_hpp

#include "1-primitives/traits/is_function.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS MEMBER FUNCTION POINTER -----------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr bool is_member_function_pointer = false;

		template <typename T, typename C>
		inline constexpr bool is_member_function_pointer<T C::*> = ca::is_function<T>;

	} // namespace impl


	/* @DOC std::is_member_function_pointer_v, as a concept. */
	template <typename T>
	concept is_member_function_pointer = ca::impl::is_member_function_pointer<ca::remove_cv<T>>;

} // namespace ca

#endif // ca_primitives_traits_is_member_function_pointer_hpp
