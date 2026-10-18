#ifndef ca_primitives_traits_is_trivially_destructible_hpp
#define ca_primitives_traits_is_trivially_destructible_hpp

#include "1-primitives/config/has_builtin.hpp"
#include "1-primitives/traits/is_destructible.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS TRIVIALLY DESTRUCTIBLE ------------------------------------------

	/* @DOC std::is_trivially_destructible_v, as a concept. */
#if ca_has_builtin(__is_trivially_destructible)
	template <typename T>
	concept is_trivially_destructible = __is_trivially_destructible(T);
#else
	template <typename T>
	concept is_trivially_destructible = ca::is_destructible<T> && __has_trivial_destructor(T);
#endif

} // namespace ca

#endif // ca_primitives_traits_is_trivially_destructible_hpp
