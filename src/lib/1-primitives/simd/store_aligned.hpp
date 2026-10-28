#ifndef ca_primitives_simd_store_aligned_hpp
#define ca_primitives_simd_store_aligned_hpp

#include "1-primitives/memory/impl/loose_bytes.hpp"


// -- CA SIMD NAMESPACE ------------------------------------------------------

namespace ca::simd {


	// -- STORE ALIGNED ------------------------------------------------------

	/* @DOC ca::simd::store to p aligned to sizeof(V): one aligned store (movdqa), which faults
	   on a misaligned p. */
	template <typename V>
	[[gnu::always_inline]] inline auto store_aligned(void* const p, const V& v) noexcept -> void {
		static_cast<ca::impl::aligned_bytes<V>*>(p)->_value = v;
	}

} // namespace ca::simd

#endif // ca_primitives_simd_store_aligned_hpp
