#ifndef ca_primitives_traits_reference_converts_from_temporary_hpp
#define ca_primitives_traits_reference_converts_from_temporary_hpp

#include "1-primitives/config/has_builtin.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/reference_constructs_from_temporary.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REFERENCE CONVERTS FROM TEMPORARY ----------------------------------

	/* @DOC std::reference_converts_from_temporary_v, as a concept. Without
	   __reference_converts_from_temporary (clang 18): reference_constructs_from_temporary &&
	   is_convertible. */
#if ca_has_builtin(__reference_converts_from_temporary)
	template <typename T, typename U> // concept-name: ignore
	concept reference_converts_from_temporary = __reference_converts_from_temporary(T, U);
#else
	template <typename T, typename U> // concept-name: ignore
	concept reference_converts_from_temporary = ca::reference_constructs_from_temporary<T, U> && ca::is_convertible<U, T>;
#endif

} // namespace ca

#endif // ca_primitives_traits_reference_converts_from_temporary_hpp
