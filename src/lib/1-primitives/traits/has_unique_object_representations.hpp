#ifndef ca_primitives_traits_has_unique_object_representations_hpp
#define ca_primitives_traits_has_unique_object_representations_hpp

#include "1-primitives/traits/remove_all_extents.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HAS UNIQUE OBJECT REPRESENTATIONS ----------------------------------

	/* @DOC std::has_unique_object_representations_v, as a concept. */
	template <typename T> // concept-name: ignore
	concept has_unique_object_representations = __has_unique_object_representations(ca::remove_cv<ca::remove_all_extents<T>>);

} // namespace ca

#endif // ca_primitives_traits_has_unique_object_representations_hpp
