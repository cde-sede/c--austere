#ifndef ca_primitives_traits_is_nothrow_invocable_r_hpp
#define ca_primitives_traits_is_nothrow_invocable_r_hpp

#include "1-primitives/traits/invoke_result.hpp"
#include "1-primitives/traits/is_nothrow_convertible.hpp"
#include "1-primitives/traits/is_nothrow_invocable.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_void.hpp"
#include "1-primitives/traits/reference_converts_from_temporary.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS NOTHROW INVOCABLE R ---------------------------------------------

	namespace impl {

		template <typename R, typename T>
		concept is_invoke_nothrow_convertible = ca::is_void<R> || ca::is_same<T, R>
			|| (ca::is_nothrow_convertible<T, R> && !ca::reference_converts_from_temporary<R, T>);

	} // namespace impl


	/* @DOC std::is_nothrow_invocable_r_v, as a concept. C++23 rule: a reference result bound to a
	   temporary is not invocable. */
	template <typename R, typename F, typename... Args>
	concept is_nothrow_invocable_r = ca::is_nothrow_invocable<F, Args...>
		&& ca::impl::is_invoke_nothrow_convertible<R, ca::invoke_result<F, Args...>>;

} // namespace ca

#endif // ca_primitives_traits_is_nothrow_invocable_r_hpp
