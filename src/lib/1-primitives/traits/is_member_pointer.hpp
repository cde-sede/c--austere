#ifndef ca_primitives_traits_is_member_pointer_hpp
#define ca_primitives_traits_is_member_pointer_hpp

#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS MEMBER POINTER --------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr bool is_member_pointer = false;

		template <typename T, typename C>
		inline constexpr bool is_member_pointer<T C::*> = true;

	} // namespace impl


	/* @DOC std::is_member_pointer_v, as a concept. */
	template <typename T>
	concept is_member_pointer = ca::impl::is_member_pointer<ca::remove_cv<T>>;

} // namespace ca

#endif // ca_primitives_traits_is_member_pointer_hpp
