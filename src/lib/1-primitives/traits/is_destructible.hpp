#ifndef ca_primitives_traits_is_destructible_hpp
#define ca_primitives_traits_is_destructible_hpp

#include "1-primitives/traits/is_function.hpp"
#include "1-primitives/traits/is_reference.hpp"
#include "1-primitives/traits/is_unbounded_array.hpp"
#include "1-primitives/traits/is_void.hpp"
#include "1-primitives/traits/remove_all_extents.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS DESTRUCTIBLE ----------------------------------------------------

	namespace impl {

		template <typename T>
		concept is_destructible_object = requires(T& t) {
			t.~T();
		};

	} // namespace impl


	/* @DOC std::is_destructible_v, as a concept. */
	template <typename T>
	concept is_destructible = ca::is_reference<T>
		|| (!ca::is_void<T> && !ca::is_function<T> && !ca::is_unbounded_array<T>
		&& ca::impl::is_destructible_object<ca::remove_all_extents<T>>);

} // namespace ca

#endif // ca_primitives_traits_is_destructible_hpp
