#ifndef ca_primitives_hash_hash_word_hpp
#define ca_primitives_hash_hash_word_hpp

#include "1-primitives/hash/impl/hash_multiply.hpp"
#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HASH WORD ----------------------------------------------------------

	/* @DOC Hash of a 64-bit value: one 64x64->128 multiply, every output bit depending on every
	   input bit. Integers of any width hash by value, so 5 and 5U agree. Not seeded per process:
	   no defence against chosen keys. */
	[[nodiscard, gnu::always_inline]] constexpr auto hash_word(const ca::u64 value) noexcept -> ca::u64 {
		return ca::impl::hash_mix(value ^ ca::impl::hash_secret[0], ca::impl::hash_secret[1]);
	}

} // namespace ca

#endif // ca_primitives_hash_hash_word_hpp
