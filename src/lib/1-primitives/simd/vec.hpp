#ifndef ca_primitives_simd_vec_hpp
#define ca_primitives_simd_vec_hpp

#include "1-primitives/bit/has_single_bit.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_arithmetic.hpp"


// -- CA SIMD NAMESPACE ------------------------------------------------------

namespace ca::simd {


	// -- VEC ----------------------------------------------------------------

	namespace impl {

		template <typename T, ca::usz N>
		struct vec_of final {
				using self = ca::simd::impl::vec_of<T, N>;
				using type [[gnu::vector_size(sizeof(T) * N)]] = T;
		}; // struct vec_of

	} // namespace impl

	/* @DOC N lanes of T in one vector register: the compiler's vector type, with its lane-wise
	   operators (+, -, *, &, |, ^, ~, <<, >>, comparisons giving lanes of all ones or zeros, ?:
	   selecting lane by lane) and subscript. N a power of two; 16 bytes is SSE2 and NEON, 32
	   AVX2. A vector wider than 16 bytes never crosses a call by value where its instructions
	   are off (the ABI differs, clang rejects it): the functions of ca::simd write their result
	   to a reference. */
	template <typename T, ca::usz N>
	requires ca::is_arithmetic<T> && (ca::has_single_bit(N))
	using vec = typename ca::simd::impl::vec_of<T, N>::type;

} // namespace ca::simd

#endif // ca_primitives_simd_vec_hpp
