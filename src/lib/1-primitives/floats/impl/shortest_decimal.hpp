#ifndef ca_primitives_floats_impl_shortest_decimal_hpp
#define ca_primitives_floats_impl_shortest_decimal_hpp

#include "1-primitives/floats/f32.hpp"
#include "1-primitives/floats/f64.hpp"
#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SHORTEST DECIMAL ---------------------------------------------------

	namespace impl {

		/* @DOC digits * 10^exponent, digits without trailing zeros only when the shortest form
		   has none. */
		struct decimal_float final {
				using self = ca::impl::decimal_float;
				ca::u64 _digits;
				ca::i32 _exponent;
		}; // struct decimal_float

		/* @DOC The decimal with the fewest digits that reads back as value (round to nearest),
		   the closest of those when several: Ryu (Ulf Adams, 2018), 128-bit powers of 5 from
		   impl/power5_tables.hpp. value finite and nonzero; its sign ignored. The f32 overload
		   uses the f32's own rounding interval, so 0.1F gives 1e-1. */
		[[nodiscard]] auto shortest_decimal(ca::f64 value) noexcept -> ca::impl::decimal_float;

		[[nodiscard]] auto shortest_decimal(ca::f32 value) noexcept -> ca::impl::decimal_float;

	} // namespace impl

} // namespace ca

#endif // ca_primitives_floats_impl_shortest_decimal_hpp
