#ifndef ca_primitives_traits_is_scalar_hpp
#define ca_primitives_traits_is_scalar_hpp

#include "1-primitives/traits/is_arithmetic.hpp"
#include "1-primitives/traits/is_enum.hpp"
#include "1-primitives/traits/is_member_pointer.hpp"
#include "1-primitives/traits/is_null_pointer.hpp"
#include "1-primitives/traits/is_pointer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS SCALAR ----------------------------------------------------------

	/* @DOC std::is_scalar_v, as a concept. */
	template <typename T>
	concept is_scalar = ca::is_arithmetic<T> || ca::is_enum<T> || ca::is_pointer<T>
		|| ca::is_member_pointer<T> || ca::is_null_pointer<T>;

} // namespace ca

#endif // ca_primitives_traits_is_scalar_hpp
