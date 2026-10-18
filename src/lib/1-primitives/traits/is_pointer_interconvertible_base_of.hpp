#ifndef ca_primitives_traits_is_pointer_interconvertible_base_of_hpp
#define ca_primitives_traits_is_pointer_interconvertible_base_of_hpp

#include "1-primitives/traits/is_base_of.hpp"
#include "1-primitives/traits/is_class.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_standard_layout.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS POINTER INTERCONVERTIBLE BASE OF --------------------------------

	/* @DOC std::is_pointer_interconvertible_base_of_v, as a concept. */
	template <typename B, typename D>
	concept is_pointer_interconvertible_base_of = (ca::is_class<B> && ca::is_same<ca::remove_cv<B>, ca::remove_cv<D>>)
		|| (ca::is_base_of<B, D> && ca::is_standard_layout<D>);

} // namespace ca

#endif // ca_primitives_traits_is_pointer_interconvertible_base_of_hpp
