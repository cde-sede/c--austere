#ifndef ca_primitives_traits_is_pointer_hpp
#define ca_primitives_traits_is_pointer_hpp

#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS POINTER ---------------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr bool is_pointer = false;

		template <typename T>
		inline constexpr bool is_pointer<T*> = true;

	} // namespace impl


	/* @DOC std::is_pointer_v, as a concept. */
	template <typename T>
	concept is_pointer = ca::impl::is_pointer<ca::remove_cv<T>>;

} // namespace ca

#endif // ca_primitives_traits_is_pointer_hpp
