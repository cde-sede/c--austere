#ifndef ca_primitives_memory_string_length_hpp
#define ca_primitives_memory_string_length_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"
#include "1-primitives/traits/is_same.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- STRING LENGTH ------------------------------------------------------

	/* @DOC Length in code units of a NUL-terminated string, NUL excluded. Folds to a constant on
	   literals. */
	template <ca::is_character C>
	[[nodiscard]] constexpr auto string_length(const C* const text) noexcept -> ca::usz {
		if constexpr (ca::is_same<C, char>)
			return __builtin_strlen(text);
		else {
			ca::usz length = 0U;
			while (text[length] != C{})
				++length;
			return length;
		}
	}

} // namespace ca

#endif // ca_primitives_memory_string_length_hpp
