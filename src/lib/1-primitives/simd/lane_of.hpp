#ifndef ca_primitives_simd_lane_of_hpp
#define ca_primitives_simd_lane_of_hpp

#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/utility/declval.hpp"


// -- CA SIMD NAMESPACE ------------------------------------------------------

namespace ca::simd {


	// -- LANE OF ------------------------------------------------------------

	/* @DOC The lane type of the vector type V: T of ca::simd::vec<T, N>. */
	template <typename V>
	using lane_of = ca::remove_cvref<decltype(ca::declval<V&>()[0])>;

} // namespace ca::simd

#endif // ca_primitives_simd_lane_of_hpp
