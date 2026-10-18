#ifndef ca_primitives_traits_is_floating_point_hpp
#define ca_primitives_traits_is_floating_point_hpp

#include "1-primitives/traits/impl/is_any_of.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS FLOATING POINT --------------------------------------------------

	/* @DOC std::is_floating_point_v, as a concept. */
	template <typename T>
	concept is_floating_point = ca::impl::is_any_of<ca::remove_cv<T>, float, double, long double>;

} // namespace ca

#endif // ca_primitives_traits_is_floating_point_hpp
