#ifndef ca_primitives_traits_is_invocable_hpp
#define ca_primitives_traits_is_invocable_hpp

#include "1-primitives/traits/invoke_result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS INVOCABLE -------------------------------------------------------

	/* @DOC std::is_invocable_v, as a concept. */
	template <typename F, typename... Args>
	concept is_invocable = requires {
		typename ca::invoke_result<F, Args...>;
	};

} // namespace ca

#endif // ca_primitives_traits_is_invocable_hpp
