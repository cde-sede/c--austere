#ifndef ca_primitives_floats_f32_hpp
#define ca_primitives_floats_f32_hpp

#include "1-primitives/integers/bits.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- F32 ----------------------------------------------------------------

	/* @DOC IEEE 754 binary32; fails to compile otherwise. */
	using f32 = float;

	static_assert(ca::bits<ca::f32> == 32U && __FLT_MANT_DIG__ == 24, "ca: f32 requires IEEE 754 binary32");

} // namespace ca

#endif // ca_primitives_floats_f32_hpp
