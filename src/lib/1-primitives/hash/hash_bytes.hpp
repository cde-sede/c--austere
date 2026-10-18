#ifndef ca_primitives_hash_hash_bytes_hpp
#define ca_primitives_hash_hash_bytes_hpp

#include "1-primitives/hash/impl/hash_multiply.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IMPL ---------------------------------------------------------------

	namespace impl {

		[[nodiscard, gnu::always_inline]] inline auto hash_read8(const ca::u8* const at) noexcept -> ca::u64 {

			ca::u64 word{0U};
			ca::copy_bytes(&word, at, sizeof(word));
			return word;
		}

		[[nodiscard, gnu::always_inline]] inline auto hash_read4(const ca::u8* const at) noexcept -> ca::u64 {

			ca::u32 word{0U};
			ca::copy_bytes(&word, at, sizeof(word));
			return word;
		}

	} // namespace impl


	// -- HASH BYTES ---------------------------------------------------------

	/* @DOC Hash of size bytes at data, from seed: wyhash (final 4). Up to 16 bytes cost two or
	   three loads and two multiplies, without a loop; longer inputs take 16 bytes per multiply,
	   three independent lanes past 48. The value depends on byte order. Not seeded per process:
	   no defence against chosen keys. */
	[[nodiscard]] inline auto hash_bytes(const void* const data, const ca::usz size, ca::u64 seed = 0U) noexcept -> ca::u64 {

		const ca::u64* const secret = ca::impl::hash_secret;
		const ca::u8* at = static_cast<const ca::u8*>(data);
		seed ^= ca::impl::hash_mix(seed ^ secret[0], secret[1]);
		ca::u64 a{0U};
		ca::u64 b{0U};
		if (size <= 16U) [[likely]] {
			if (size >= 4U) {
				const ca::usz shift = (size >> 3U) << 2U;
				a = (ca::impl::hash_read4(at) << 32U) | ca::impl::hash_read4(at + shift);
				b = (ca::impl::hash_read4(at + size - 4U) << 32U) | ca::impl::hash_read4(at + size - 4U - shift);
			}
			else if (size > 0U)
				a = (static_cast<ca::u64>(at[0]) << 16U) | (static_cast<ca::u64>(at[size >> 1U]) << 8U) | at[size - 1U];
		}
		else {
			ca::usz left = size;
			if (left > 48U) {
				ca::u64 lane1 = seed;
				ca::u64 lane2 = seed;
				do {
					 seed = ca::impl::hash_mix(ca::impl::hash_read8(at) ^ secret[1], ca::impl::hash_read8(at + 8U) ^ seed);
					lane1 = ca::impl::hash_mix(ca::impl::hash_read8(at + 16U) ^ secret[2], ca::impl::hash_read8(at + 24U) ^ lane1);
					lane2 = ca::impl::hash_mix(ca::impl::hash_read8(at + 32U) ^ secret[3], ca::impl::hash_read8(at + 40U) ^ lane2);
					at += 48U;
					left -= 48U;
				} while (left > 48U);
				seed ^= lane1 ^ lane2;
			}
			while (left > 16U) {
				seed = ca::impl::hash_mix(ca::impl::hash_read8(at) ^ secret[1], ca::impl::hash_read8(at + 8U) ^ seed);
				at += 16U;
				left -= 16U;
			}
			a = ca::impl::hash_read8(at + left - 16U);
			b = ca::impl::hash_read8(at + left - 8U);
		}
		a ^= secret[1];
		b ^= seed;
		ca::impl::hash_multiply(a, b);
		return ca::impl::hash_mix(a ^ secret[0] ^ size, b ^ secret[1]);
	}

} // namespace ca

#endif // ca_primitives_hash_hash_bytes_hpp
