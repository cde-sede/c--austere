#ifndef ca_primitives_simd_splat_hpp
#define ca_primitives_simd_splat_hpp

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/simd/lane_of.hpp"
#include "1-primitives/simd/vec.hpp"
#include "1-primitives/traits/is_arithmetic.hpp"


// -- CA SIMD NAMESPACE ------------------------------------------------------

namespace ca::simd {


	// -- SPLAT --------------------------------------------------------------

	namespace impl {

#if ca_architecture(x86_64)

		/* @DOC The low byte of value in every lane of v: vmovd and vpbroadcastb. A 32-bit word
		   broadcast puts an imul on the path to the first use (memchr 16 - 256 B 7 - 13%
		   slower); V{} + value is vpinsrb lane by lane on gcc. */
		template <typename V>
		requires (sizeof(V) == 32U)
		[[gnu::target("avx2")]] inline auto broadcast_byte(const int value, V& v) noexcept -> void {

			using words [[gnu::vector_size(16)]] = int;
			using bytes = ca::simd::vec<char, 16U>;
			const bytes low = __builtin_bit_cast(bytes, words{value, 0, 0, 0});
			v = __builtin_bit_cast(V, __builtin_shufflevector(low, low, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
		}

#endif

	} // namespace impl

	/* @DOC value, converted to the lane type, in every lane of v. Byte lanes take the low byte
	   of any integer: 16 of them are a 32-bit word pattern (gcc builds V{} + value one vpinsrb at
	   a time), 32 a vpbroadcastb in AVX2 code. */
	template <typename V, typename T>
	requires ca::is_arithmetic<T>
	[[gnu::always_inline]] constexpr auto splat(const T value, V& v) noexcept -> void {

		using lane = ca::simd::lane_of<V>;
		if consteval {
			v = V{} + static_cast<lane>(value);
		} else {
			if constexpr (sizeof(lane) == 1U && sizeof(V) % 4U == 0U && sizeof(V) <= 16U) {
				using words [[gnu::vector_size(sizeof(V))]] = ca::u32;
				v = __builtin_bit_cast(V, words{} + static_cast<ca::u8>(value) * 0x01010101U);
			}
#if ca_architecture(x86_64)
			else if constexpr (sizeof(lane) == 1U && sizeof(V) == 32U)
				ca::simd::impl::broadcast_byte(static_cast<int>(value), v);
#endif
			else
				v = V{} + static_cast<lane>(value);
		}
	}

} // namespace ca::simd

#endif // ca_primitives_simd_splat_hpp
