#ifndef ca_primitives_bit_countl_zero_hpp
#define ca_primitives_bit_countl_zero_hpp

#include "1-primitives/bit/impl/is_bit_unsigned.hpp"
#include "1-primitives/integers/bits.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COUNTL ZERO --------------------------------------------------------

	/* @DOC Consecutive 0 bits from the most significant end. Unsigned integers only. */
	template <typename T>
	requires ca::impl::is_bit_unsigned<T>
	[[nodiscard]] constexpr auto countl_zero(const T value) noexcept -> int {

		if (value == 0U)
			return static_cast<int>(ca::bits<T>);
		if constexpr (sizeof(T) <= sizeof(unsigned int))
			return __builtin_clz(value) - static_cast<int>(ca::bits<unsigned int> - ca::bits<T>);
		else if constexpr (sizeof(T) <= sizeof(unsigned long))
			return __builtin_clzl(value) - static_cast<int>(ca::bits<unsigned long> - ca::bits<T>);
		else
			return __builtin_clzll(value) - static_cast<int>(ca::bits<unsigned long long> - ca::bits<T>);
	}

} // namespace ca

#endif // ca_primitives_bit_countl_zero_hpp
