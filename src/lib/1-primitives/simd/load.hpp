#ifndef ca_primitives_simd_load_hpp
#define ca_primitives_simd_load_hpp

#include "1-primitives/memory/impl/loose_bytes.hpp"


// -- CA SIMD NAMESPACE ------------------------------------------------------

namespace ca::simd {


	// -- LOAD ---------------------------------------------------------------

	/* @DOC The sizeof(V) bytes at p into v: any alignment, any type of the memory there. Every
	   byte must be in a live object; to read past a range, ca::simd::peek. */
	template <typename V>
	[[gnu::always_inline]] inline auto load(const void* const p, V& v) noexcept -> void {
		v = ca::impl::loose<V>(static_cast<const char*>(p))._value;
	}

} // namespace ca::simd

#endif // ca_primitives_simd_load_hpp
