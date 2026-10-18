#ifndef ca_primitives_traits_impl_is_any_of_hpp
#define ca_primitives_traits_impl_is_any_of_hpp

#include "1-primitives/traits/is_same.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS ANY OF ----------------------------------------------------------

	namespace impl {

		template <typename T, typename... Ts>
		concept is_any_of = (ca::is_same<T, Ts> || ...);

	} // namespace impl

} // namespace ca

#endif // ca_primitives_traits_impl_is_any_of_hpp
