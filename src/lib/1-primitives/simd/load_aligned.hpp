#ifndef ca_primitives_simd_load_aligned_hpp
#define ca_primitives_simd_load_aligned_hpp

#include "1-primitives/memory/impl/loose_bytes.hpp"


// -- CA SIMD NAMESPACE ------------------------------------------------------

namespace ca::simd {


	// -- LOAD ALIGNED -------------------------------------------------------

	/* @DOC ca::simd::load from p aligned to sizeof(V): one aligned load (movdqa), which faults
	   on a misaligned p. */
	template <typename V>
	[[gnu::always_inline]] inline auto load_aligned(const void* const p, V& v) noexcept -> void {
		v = static_cast<const ca::impl::aligned_bytes<V>*>(p)->_value;
	}

} // namespace ca::simd

#endif // ca_primitives_simd_load_aligned_hpp
