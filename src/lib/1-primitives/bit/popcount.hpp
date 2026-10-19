#ifndef ca_primitives_bit_popcount_hpp
#define ca_primitives_bit_popcount_hpp

#include "1-primitives/bit/impl/is_bit_unsigned.hpp"
#include "1-primitives/config/compiler.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- POPCOUNT -----------------------------------------------------------

	/* @DOC Number of 1 bits. Unsigned integers only. One popcnt where the target has it; else
	   the SWAR sum (clang's own expansion of the builtin): gcc would call libgcc's __popcountdi2,
	   which programs linked without libc do not have. */
	template <typename T>
	requires ca::impl::is_bit_unsigned<T>
	[[nodiscard]] constexpr auto popcount(const T value) noexcept -> int {

#if defined(__POPCNT__) || ca_compiler(clang)
		if constexpr (sizeof(T) <= sizeof(unsigned int))
			return __builtin_popcount(value);
		else if constexpr (sizeof(T) <= sizeof(unsigned long))
			return __builtin_popcountl(value);
		else
			return __builtin_popcountll(value);
#else
		unsigned long long bits = value;
		bits = bits - ((bits >> 1U) & 0x5555555555555555ULL);
		bits = (bits & 0x3333333333333333ULL) + ((bits >> 2U) & 0x3333333333333333ULL);
		bits = (bits + (bits >> 4U)) & 0x0f0f0f0f0f0f0f0fULL;
		return static_cast<int>((bits * 0x0101010101010101ULL) >> 56U);
#endif
	}

} // namespace ca

#endif // ca_primitives_bit_popcount_hpp
