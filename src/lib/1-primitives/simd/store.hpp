#ifndef ca_primitives_simd_store_hpp
#define ca_primitives_simd_store_hpp

#include "1-primitives/memory/impl/loose_bytes.hpp"


// -- CA SIMD NAMESPACE ------------------------------------------------------

namespace ca::simd {


	// -- STORE --------------------------------------------------------------

	/* @DOC v into the sizeof(V) bytes at p: any alignment, any type of the memory there. */
	template <typename V>
	[[gnu::always_inline]] inline auto store(void* const p, const V& v) noexcept -> void {
		ca::impl::loose<V>(static_cast<char*>(p))._value = v;
	}

} // namespace ca::simd

#endif // ca_primitives_simd_store_hpp
