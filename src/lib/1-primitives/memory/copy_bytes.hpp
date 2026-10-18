#ifndef ca_primitives_memory_copy_bytes_hpp
#define ca_primitives_memory_copy_bytes_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COPY BYTES ---------------------------------------------------------

	/* @DOC Copies count bytes from `from` to `to`. Ranges must not overlap; count 0 accepts any
	   pointers. The compiler's builtin, so the optimizer folds, inlines and expands around it
	   as around memcpy; what it leaves calls the C memcpy, which ca defines on x86-64
	   (ca::impl::move_bytes_variant), libc's elsewhere. */
	inline auto copy_bytes(void* const to, const void* const from, const ca::usz count) noexcept -> void {
		if (count != 0U)
			__builtin_memcpy(to, from, count);
	}

} // namespace ca

#endif // ca_primitives_memory_copy_bytes_hpp
