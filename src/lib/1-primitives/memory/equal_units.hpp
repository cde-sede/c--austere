#ifndef ca_primitives_memory_equal_units_hpp
#define ca_primitives_memory_equal_units_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"
#include "1-primitives/traits/is_constant_evaluated.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- EQUAL UNITS --------------------------------------------------------

	/* @DOC Whether count code units at left and right are equal: one memcmp outside constant
	   evaluation, whatever the unit size. */
	template <ca::is_character C>
	[[nodiscard]] constexpr auto equal_units(const C* const left, const C* const right, const ca::usz count) noexcept -> bool {

		if (!ca::is_constant_evaluated())
			return __builtin_memcmp(left, right, count * sizeof(C)) == 0;
		for (ca::usz i = 0U; i < count; ++i) {
			if (left[i] != right[i])
				return false;
		}
		return true;
	}

} // namespace ca

#endif // ca_primitives_memory_equal_units_hpp
