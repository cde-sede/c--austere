#ifndef ca_primitives_traits_is_implicit_lifetime_hpp
#define ca_primitives_traits_is_implicit_lifetime_hpp

#include "1-primitives/config/has_builtin.hpp"
#include "1-primitives/traits/is_aggregate.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_class.hpp"
#include "1-primitives/traits/is_scalar.hpp"
#include "1-primitives/traits/is_trivially_copy_constructible.hpp"
#include "1-primitives/traits/is_trivially_default_constructible.hpp"
#include "1-primitives/traits/is_trivially_destructible.hpp"
#include "1-primitives/traits/is_trivially_move_constructible.hpp"
#include "1-primitives/traits/is_union.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS IMPLICIT LIFETIME -----------------------------------------------

	/* @DOC std::is_implicit_lifetime_v, as a concept. Without __builtin_is_implicit_lifetime:
	   approximation; aggregates whose destructor is non-trivial but not user-provided report false. */
#if ca_has_builtin(__builtin_is_implicit_lifetime)
	template <typename T>
	concept is_implicit_lifetime = __builtin_is_implicit_lifetime(T);
#else
	template <typename T>
	concept is_implicit_lifetime = ca::is_scalar<T> || ca::is_array<T>
		|| ((ca::is_class<T> || ca::is_union<T>) && ca::is_trivially_destructible<T>
		&& (ca::is_aggregate<T> || ca::is_trivially_default_constructible<T>
		|| ca::is_trivially_copy_constructible<T> || ca::is_trivially_move_constructible<T>));
#endif

} // namespace ca

#endif // ca_primitives_traits_is_implicit_lifetime_hpp
