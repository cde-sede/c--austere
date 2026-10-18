#ifndef ca_collections_map_hash_hpp
#define ca_collections_map_hash_hpp

#include "1-primitives/hash/hash_bytes.hpp"
#include "1-primitives/hash/hash_word.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_enum.hpp"
#include "1-primitives/traits/is_pointer.hpp"
#include "1-primitives/traits/underlying_type.hpp"
#include "5-collections/map/is_hashable.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HASH ---------------------------------------------------------------

	/* @DOC Hash of value, what hashmap and set place it by: value.hash() when declared; integers
	   and enums by value through hash_word (every width agrees); pointers by address; text by its
	   code units through hash_bytes, so a str hashes as its sv. */
	template <ca::is_hashable T>
	[[nodiscard, gnu::always_inline]] inline auto hash(const T& value) noexcept -> ca::u64 {

		if constexpr (ca::impl::has_hash_member<T>)
			return value.hash();
		else if constexpr (ca::impl::is_hashed_text<T>)
			return ca::hash_bytes(value.data(), value.size() * sizeof(typename T::value_type));
		else if constexpr (ca::impl::has_hashed_text<T>)
			return ca::hash(value.view());
		else if constexpr (ca::is_enum<T>)
			return ca::hash_word(static_cast<ca::u64>(static_cast<ca::underlying_type<T>>(value)));
		else if constexpr (ca::is_pointer<T>)
			return ca::hash_word(static_cast<ca::u64>(reinterpret_cast<ca::usz>(value)));
		else
			return ca::hash_word(static_cast<ca::u64>(value));
	}

} // namespace ca

#endif // ca_collections_map_hash_hpp
