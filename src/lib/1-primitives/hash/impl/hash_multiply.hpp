#ifndef ca_primitives_hash_impl_hash_multiply_hpp
#define ca_primitives_hash_impl_hash_multiply_hpp

#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IMPL ---------------------------------------------------------------

	namespace impl {


		// -- HASH MULTIPLY --------------------------------------------------

		/* @DOC wyhash's secret: odd constants with balanced bits. */
		inline constexpr ca::u64 hash_secret[4] = {
			0x2d358dccaa6c78a5U, 0x8bb84b93962eacc9U, 0x4b33a62ed433d4a3U, 0x4d5a2da51de1aa47U,
		};

		/* @DOC The full 128-bit product of left and right: low half into left, high half into
		   right. */
		[[gnu::always_inline]] constexpr auto hash_multiply(ca::u64& left, ca::u64& right) noexcept -> void {

#if defined(__SIZEOF_INT128__)
			__extension__ typedef unsigned __int128 wide;
			const wide product = static_cast<wide>(left) * right;
			 left = static_cast<ca::u64>(product);
			right = static_cast<ca::u64>(product >> 64U);
#else
			const ca::u64 lh = left >> 32U;
			const ca::u64 ll = left & 0xFFFFFFFFU;
			const ca::u64 rh = right >> 32U;
			const ca::u64 rl = right & 0xFFFFFFFFU;
			const ca::u64 hh = lh * rh;
			const ca::u64 hl = lh * rl;
			const ca::u64 lr = ll * rh;
			const ca::u64 low = ll * rl;
			const ca::u64 mid = (low >> 32U) + (hl & 0xFFFFFFFFU) + (lr & 0xFFFFFFFFU);
			 left = (mid << 32U) | (low & 0xFFFFFFFFU);
			right = hh + (hl >> 32U) + (lr >> 32U) + (mid >> 32U);
#endif
		}

		/* @DOC Folds the 128-bit product of left and right into 64 bits: low ^ high. */
		[[nodiscard, gnu::always_inline]] constexpr auto hash_mix(ca::u64 left, ca::u64 right) noexcept -> ca::u64 {

			ca::impl::hash_multiply(left, right);
			return left ^ right;
		}

	} // namespace impl

} // namespace ca

#endif // ca_primitives_hash_impl_hash_multiply_hpp
