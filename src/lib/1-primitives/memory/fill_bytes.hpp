#ifndef ca_primitives_memory_fill_bytes_hpp
#define ca_primitives_memory_fill_bytes_hpp

#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FILL BYTES ---------------------------------------------------------

	/* @DOC Sets count bytes at `to` to value; count 0 accepts any pointer. */
	inline auto fill_bytes(void* const to, const ca::u8 value, const ca::usz count) noexcept -> void {
		if (count != 0U)
			__builtin_memset(to, value, count);
	}

} // namespace ca

#endif // ca_primitives_memory_fill_bytes_hpp
