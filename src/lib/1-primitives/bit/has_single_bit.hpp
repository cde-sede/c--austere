#ifndef ca_primitives_bit_has_single_bit_hpp
#define ca_primitives_bit_has_single_bit_hpp

#include "1-primitives/bit/impl/is_bit_unsigned.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HAS SINGLE BIT -----------------------------------------------------

	/* @DOC Whether value is a power of two. Unsigned integers only. */
	template <typename T>
	requires ca::impl::is_bit_unsigned<T>
	[[nodiscard]] constexpr auto has_single_bit(const T value) noexcept -> bool {
		return value != 0U && (value & (value - 1U)) == 0U;
	}

} // namespace ca

#endif // ca_primitives_bit_has_single_bit_hpp
