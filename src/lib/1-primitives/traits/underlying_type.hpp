#ifndef ca_primitives_traits_underlying_type_hpp
#define ca_primitives_traits_underlying_type_hpp

#include "1-primitives/traits/is_enum.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNDERLYING TYPE ----------------------------------------------------

	/* @DOC std::underlying_type_t. */
	template <typename T>
	requires ca::is_enum<T>
	using underlying_type = __underlying_type(T);

} // namespace ca

#endif // ca_primitives_traits_underlying_type_hpp
