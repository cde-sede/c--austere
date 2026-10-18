#ifndef ca_primitives_bit_countr_one_hpp
#define ca_primitives_bit_countr_one_hpp

#include "1-primitives/bit/countr_zero.hpp"
#include "1-primitives/bit/impl/is_bit_unsigned.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COUNTR ONE ---------------------------------------------------------

	/* @DOC Consecutive 1 bits from the least significant end. Unsigned integers only. */
	template <typename T>
	requires ca::impl::is_bit_unsigned<T>
	[[nodiscard]] constexpr auto countr_one(const T value) noexcept -> int {
		return ca::countr_zero(static_cast<T>(~value));
	}

} // namespace ca

#endif // ca_primitives_bit_countr_one_hpp
