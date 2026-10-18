#ifndef ca_primitives_traits_is_aggregate_hpp
#define ca_primitives_traits_is_aggregate_hpp

#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS AGGREGATE -------------------------------------------------------

	/* @DOC std::is_aggregate_v, as a concept. */
	template <typename T>
	concept is_aggregate = __is_aggregate(ca::remove_cv<T>);

} // namespace ca

#endif // ca_primitives_traits_is_aggregate_hpp
