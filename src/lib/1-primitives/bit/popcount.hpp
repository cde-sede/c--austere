#ifndef ca_primitives_bit_popcount_hpp
#define ca_primitives_bit_popcount_hpp

#include "1-primitives/bit/impl/is_bit_unsigned.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- POPCOUNT -----------------------------------------------------------

	/* @DOC Number of 1 bits. Unsigned integers only. */
	template <typename T>
	requires ca::impl::is_bit_unsigned<T>
	[[nodiscard]] constexpr auto popcount(const T value) noexcept -> int {

		if constexpr (sizeof(T) <= sizeof(unsigned int))
			return __builtin_popcount(value);
		else if constexpr (sizeof(T) <= sizeof(unsigned long))
			return __builtin_popcountl(value);
		else
			return __builtin_popcountll(value);
	}

} // namespace ca

#endif // ca_primitives_bit_popcount_hpp
