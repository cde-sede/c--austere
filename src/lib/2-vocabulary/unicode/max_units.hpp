#ifndef ca_vocabulary_unicode_max_units_hpp
#define ca_vocabulary_unicode_max_units_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- MAX UNITS ------------------------------------------------------

		/* @DOC Most code units one code point takes in C's encoding: 4 for UTF-8, 2 for UTF-16, 1
		   for UTF-32. */
		template <ca::is_character C>
		inline constexpr ca::usz max_units = sizeof(C) == 1U ? 4U : sizeof(C) == 2U ? 2U : 1U;

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_max_units_hpp
