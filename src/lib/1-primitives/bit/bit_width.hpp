#ifndef ca_primitives_bit_bit_width_hpp
#define ca_primitives_bit_bit_width_hpp

#include "1-primitives/bit/countl_zero.hpp"
#include "1-primitives/bit/impl/is_bit_unsigned.hpp"
#include "1-primitives/integers/bits.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BIT WIDTH ----------------------------------------------------------

	/* @DOC Bits needed to represent value; 0 for 0. Unsigned integers only. */
	template <typename T>
	requires ca::impl::is_bit_unsigned<T>
	[[nodiscard]] constexpr auto bit_width(const T value) noexcept -> int {
		return static_cast<int>(ca::bits<T>) - ca::countl_zero(value);
	}

} // namespace ca

#endif // ca_primitives_bit_bit_width_hpp
