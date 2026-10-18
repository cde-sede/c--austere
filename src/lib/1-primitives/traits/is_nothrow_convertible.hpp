#ifndef ca_primitives_traits_is_nothrow_convertible_hpp
#define ca_primitives_traits_is_nothrow_convertible_hpp

#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_void.hpp"
#include "1-primitives/utility/declval.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS NOTHROW CONVERTIBLE ---------------------------------------------

	namespace impl {

		template <typename T>
		auto convert_to(T) noexcept -> void;

	} // namespace impl


	/* @DOC std::is_nothrow_convertible_v, as a concept. */
	template <typename From, typename To>
	concept is_nothrow_convertible = ca::is_convertible<From, To>
		&& (ca::is_void<To> || noexcept(ca::impl::convert_to<To>(ca::declval<From>())));

} // namespace ca

#endif // ca_primitives_traits_is_nothrow_convertible_hpp
