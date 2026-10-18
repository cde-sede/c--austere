#ifndef ca_primitives_memory_compare_units_hpp
#define ca_primitives_memory_compare_units_hpp

#include "1-primitives/integers/uint_of.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"
#include "1-primitives/traits/is_constant_evaluated.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COMPARE UNITS ------------------------------------------------------

	/* @DOC Lexicographic comparison of count code units, each as unsigned (code point order for
	   UTF-8 and UTF-32): negative, 0 or positive. memcmp for 1-byte units. */
	template <ca::is_character C>
	[[nodiscard]] constexpr auto compare_units(const C* const left, const C* const right, const ca::usz count) noexcept -> int {

		using unit = ca::uint_of<sizeof(C) * 8U>;
		if constexpr (sizeof(C) == 1U) {
			if (!ca::is_constant_evaluated())
				return __builtin_memcmp(left, right, count);
		}
		for (ca::usz i = 0U; i < count; ++i) {
			if (left[i] != right[i])
				return static_cast<unit>(left[i]) < static_cast<unit>(right[i]) ? -1 : 1;
		}
		return 0;
	}

} // namespace ca

#endif // ca_primitives_memory_compare_units_hpp
