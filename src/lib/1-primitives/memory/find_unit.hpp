#ifndef ca_primitives_memory_find_unit_hpp
#define ca_primitives_memory_find_unit_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"
#include "1-primitives/traits/is_constant_evaluated.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FIND UNIT ----------------------------------------------------------

	/* @DOC First of the count code units at data equal to value, or null. memchr for 1-byte units
	   outside constant evaluation. */
	template <ca::is_character C>
	[[nodiscard]] constexpr auto find_unit(const C* const data, const C value, const ca::usz count) noexcept -> const C* {

		if constexpr (sizeof(C) == 1U) {
			if (!ca::is_constant_evaluated())
				return count == 0U ? nullptr : static_cast<const C*>(__builtin_memchr(data, static_cast<unsigned char>(value), count));
		}
		for (ca::usz i = 0U; i < count; ++i) {
			if (data[i] == value)
				return data + i;
		}
		return nullptr;
	}

} // namespace ca

#endif // ca_primitives_memory_find_unit_hpp
