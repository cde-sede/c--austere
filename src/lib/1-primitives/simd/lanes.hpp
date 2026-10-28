#ifndef ca_primitives_simd_lanes_hpp
#define ca_primitives_simd_lanes_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/simd/lane_of.hpp"


// -- CA SIMD NAMESPACE ------------------------------------------------------

namespace ca::simd {


	// -- LANES --------------------------------------------------------------

	/* @DOC The lane count of the vector type V: N of ca::simd::vec<T, N>. */
	template <typename V>
	inline constexpr ca::usz lanes = sizeof(V) / sizeof(ca::simd::lane_of<V>);

} // namespace ca::simd

#endif // ca_primitives_simd_lanes_hpp
