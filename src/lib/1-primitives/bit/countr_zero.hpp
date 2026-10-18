#ifndef ca_primitives_bit_countr_zero_hpp
#define ca_primitives_bit_countr_zero_hpp

#include "1-primitives/bit/impl/is_bit_unsigned.hpp"
#include "1-primitives/integers/bits.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COUNTR ZERO --------------------------------------------------------

	/* @DOC Consecutive 0 bits from the least significant end. Unsigned integers only. */
	template <typename T>
	requires ca::impl::is_bit_unsigned<T>
	[[nodiscard]] constexpr auto countr_zero(const T value) noexcept -> int {

		if (value == 0U)
			return static_cast<int>(ca::bits<T>);
		if constexpr (sizeof(T) <= sizeof(unsigned int))
			return __builtin_ctz(value);
		else if constexpr (sizeof(T) <= sizeof(unsigned long))
			return __builtin_ctzl(value);
		else
			return __builtin_ctzll(value);
	}

} // namespace ca

#endif // ca_primitives_bit_countr_zero_hpp
