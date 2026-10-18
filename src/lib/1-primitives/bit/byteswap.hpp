#ifndef ca_primitives_bit_byteswap_hpp
#define ca_primitives_bit_byteswap_hpp

#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/traits/is_integral.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BYTESWAP -----------------------------------------------------------

	/* @DOC Reverses the byte order of an integer. */
	template <typename T>
	requires ca::is_integral<T>
	[[nodiscard]] constexpr auto byteswap(const T value) noexcept -> T {

		if constexpr (sizeof(T) == 1U)
			return value;
		else if constexpr (sizeof(T) == 2U)
			return static_cast<T>(__builtin_bswap16(static_cast<ca::u16>(value)));
		else if constexpr (sizeof(T) == 4U)
			return static_cast<T>(__builtin_bswap32(static_cast<ca::u32>(value)));
		else
			return static_cast<T>(__builtin_bswap64(static_cast<ca::u64>(value)));
	}

} // namespace ca

#endif // ca_primitives_bit_byteswap_hpp
