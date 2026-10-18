#ifndef ca_primitives_traits_impl_is_referenceable_hpp
#define ca_primitives_traits_impl_is_referenceable_hpp

#include "1-primitives/traits/type_identity.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS REFERENCEABLE ---------------------------------------------------

	namespace impl {

		template <typename T>
		concept is_referenceable = requires {
			typename ca::type_identity<T&>;
		};

	} // namespace impl

} // namespace ca

#endif // ca_primitives_traits_impl_is_referenceable_hpp
