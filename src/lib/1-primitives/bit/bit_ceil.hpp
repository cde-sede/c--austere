#ifndef ca_primitives_bit_bit_ceil_hpp
#define ca_primitives_bit_bit_ceil_hpp

#include "1-primitives/bit/bit_width.hpp"
#include "1-primitives/bit/impl/is_bit_unsigned.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BIT CEIL -----------------------------------------------------------

	/* @DOC Smallest power of two not below value; undefined when not representable. Unsigned
	   integers only. */
	template <typename T>
	requires ca::impl::is_bit_unsigned<T>
	[[nodiscard]] constexpr auto bit_ceil(const T value) noexcept -> T {

		if (value <= 1U)
			return 1U;
		return static_cast<T>(static_cast<T>(1U) << ca::bit_width(static_cast<T>(value - 1U)));
	}

} // namespace ca

#endif // ca_primitives_bit_bit_ceil_hpp
