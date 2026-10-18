#ifndef ca_primitives_memory_compare_bytes_hpp
#define ca_primitives_memory_compare_bytes_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COMPARE BYTES ------------------------------------------------------

	/* @DOC Lexicographic comparison of count chars, as unsigned: negative, 0 or positive. Usable in
	   constant expressions. */
	[[nodiscard]] constexpr auto compare_bytes(const char* const left, const char* const right, const ca::usz count) noexcept -> int {
		return __builtin_memcmp(left, right, count);
	}

} // namespace ca

#endif // ca_primitives_memory_compare_bytes_hpp
