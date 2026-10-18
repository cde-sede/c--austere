#ifndef ca_primitives_bit_bit_cast_hpp
#define ca_primitives_bit_bit_cast_hpp

#include "1-primitives/traits/is_trivially_copyable.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BIT CAST -----------------------------------------------------------

	/* @DOC Object representation of from, as a To. Same size, both trivially copyable. */
	template <typename To, typename From>
	requires (sizeof(To) == sizeof(From) && ca::is_trivially_copyable<To> && ca::is_trivially_copyable<From>)
	[[nodiscard]] constexpr auto bit_cast(const From& from) noexcept -> To {
		return __builtin_bit_cast(To, from);
	}

} // namespace ca

#endif // ca_primitives_bit_bit_cast_hpp
