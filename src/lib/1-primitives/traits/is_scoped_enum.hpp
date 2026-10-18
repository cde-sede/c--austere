#ifndef ca_primitives_traits_is_scoped_enum_hpp
#define ca_primitives_traits_is_scoped_enum_hpp

#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_enum.hpp"
#include "1-primitives/traits/underlying_type.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS SCOPED ENUM -----------------------------------------------------

	/* @DOC std::is_scoped_enum_v, as a concept. */
	template <typename T>
	concept is_scoped_enum = ca::is_enum<T> && !ca::is_convertible<T, ca::underlying_type<T>>;

} // namespace ca

#endif // ca_primitives_traits_is_scoped_enum_hpp
