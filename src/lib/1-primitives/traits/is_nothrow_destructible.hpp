#ifndef ca_primitives_traits_is_nothrow_destructible_hpp
#define ca_primitives_traits_is_nothrow_destructible_hpp

#include "1-primitives/traits/is_destructible.hpp"
#include "1-primitives/traits/is_reference.hpp"
#include "1-primitives/traits/remove_all_extents.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS NOTHROW DESTRUCTIBLE --------------------------------------------

	namespace impl {

		template <typename T>
		concept is_nothrow_destructible_object = requires(T& t) {
			{ t.~T() } noexcept;
		};

	} // namespace impl


	/* @DOC std::is_nothrow_destructible_v, as a concept. */
	template <typename T>
	concept is_nothrow_destructible = ca::is_reference<T>
		|| (ca::is_destructible<T> && ca::impl::is_nothrow_destructible_object<ca::remove_all_extents<T>>);

} // namespace ca

#endif // ca_primitives_traits_is_nothrow_destructible_hpp
