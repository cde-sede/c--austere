#ifndef ca_primitives_memory_find_byte_hpp
#define ca_primitives_memory_find_byte_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FIND BYTE ----------------------------------------------------------

	/* @DOC First byte equal to value among the count bytes at data, or null. count 0 accepts any
	   pointer. */
	[[nodiscard]] inline auto find_byte(const char* const data, const char value, const ca::usz count) noexcept -> const char* {

		if (count == 0U)
			return nullptr;
		return static_cast<const char*>(__builtin_memchr(data, value, count));
	}

} // namespace ca

#endif // ca_primitives_memory_find_byte_hpp
