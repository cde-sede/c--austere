#ifndef ca_primitives_bit_bit_floor_hpp
#define ca_primitives_bit_bit_floor_hpp

#include "1-primitives/bit/bit_width.hpp"
#include "1-primitives/bit/impl/is_bit_unsigned.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BIT FLOOR ----------------------------------------------------------

	/* @DOC Largest power of two not above value; 0 for 0. Unsigned integers only. */
	template <typename T>
	requires ca::impl::is_bit_unsigned<T>
	[[nodiscard]] constexpr auto bit_floor(const T value) noexcept -> T {

		if (value == 0U)
			return 0U;
		return static_cast<T>(static_cast<T>(1U) << (ca::bit_width(value) - 1));
	}

} // namespace ca

#endif // ca_primitives_bit_bit_floor_hpp
