#ifndef ca_primitives_traits_is_nothrow_invocable_hpp
#define ca_primitives_traits_is_nothrow_invocable_hpp

#include "1-primitives/traits/invoke_result.hpp"
#include "1-primitives/traits/is_invocable.hpp"
#include "1-primitives/utility/declval.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS NOTHROW INVOCABLE -----------------------------------------------

	/* @DOC std::is_nothrow_invocable_v, as a concept. */
	template <typename F, typename... Args>
	concept is_nothrow_invocable = ca::is_invocable<F, Args...>
		&& noexcept(ca::impl::invoke(ca::declval<F>(), ca::declval<Args>()...));

} // namespace ca

#endif // ca_primitives_traits_is_nothrow_invocable_hpp
