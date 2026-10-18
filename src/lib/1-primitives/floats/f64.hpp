#ifndef ca_primitives_floats_f64_hpp
#define ca_primitives_floats_f64_hpp

#include "1-primitives/integers/bits.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- F64 ----------------------------------------------------------------

	/* @DOC IEEE 754 binary64; fails to compile otherwise. */
	using f64 = double;

	static_assert(ca::bits<ca::f64> == 64U && __DBL_MANT_DIG__ == 53, "ca: f64 requires IEEE 754 binary64");

} // namespace ca

#endif // ca_primitives_floats_f64_hpp
