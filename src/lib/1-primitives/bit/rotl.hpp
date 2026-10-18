#ifndef ca_primitives_bit_rotl_hpp
#define ca_primitives_bit_rotl_hpp

#include "1-primitives/bit/impl/is_bit_unsigned.hpp"
#include "1-primitives/integers/bits.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ROTL ---------------------------------------------------------------

	/* @DOC Rotates left by shift; a negative shift rotates right. Unsigned integers only. */
	template <typename T>
	requires ca::impl::is_bit_unsigned<T>
	[[nodiscard]] constexpr auto rotl(const T value, const int shift) noexcept -> T {

		constexpr int n = static_cast<int>(ca::bits<T>);
		int r = shift % n;
		if (r < 0)
			r += n;
		if (r == 0)
			return value;
		return static_cast<T>((value << r) | (value >> (n - r)));
	}

} // namespace ca

#endif // ca_primitives_bit_rotl_hpp
