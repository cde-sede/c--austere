#ifndef ca_primitives_traits_impl_copy_cv_hpp
#define ca_primitives_traits_impl_copy_cv_hpp

#include "1-primitives/traits/conditional.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_volatile.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COPY CV ------------------------------------------------------------

	namespace impl {

		template <typename T, typename U>
		using copy_cv = ca::conditional<ca::is_const<T>,
			ca::conditional<ca::is_volatile<T>, const volatile U, const U>,
			ca::conditional<ca::is_volatile<T>, volatile U, U>>;

	} // namespace impl

} // namespace ca

#endif // ca_primitives_traits_impl_copy_cv_hpp
