#ifndef ca_primitives_floats_impl_exact_decimal_hpp
#define ca_primitives_floats_impl_exact_decimal_hpp

#include "1-primitives/floats/f64.hpp"
#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- EXACT DECIMAL ------------------------------------------------------

	namespace impl {

		/* @DOC Every decimal digit of an f64: value = 0.d1 d2 ... d_count * 10^_point, d1 not
		   zero. A double has at most 767 significant digits (subnormals). */
		struct exact_decimal final {
				using self = ca::impl::exact_decimal;
				static constexpr ca::usz CAPACITY = 800U;
				char _digits[CAPACITY];
				ca::usz _count;
				ca::i32 _point;
		}; // struct exact_decimal

		/* @DOC Writes the exact decimal expansion of value (finite, nonzero; sign ignored) into
		   out: value = m * 2^e becomes the integer m << e, or m * 5^-e with the point -e places
		   in, in 32-bit limbs, read out 9 digits per division by 10^9. */
		auto exact_decimal_of(ca::f64 value, ca::impl::exact_decimal& out) noexcept -> void;

	} // namespace impl

} // namespace ca

#endif // ca_primitives_floats_impl_exact_decimal_hpp
