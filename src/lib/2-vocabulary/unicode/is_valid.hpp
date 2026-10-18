#ifndef ca_vocabulary_unicode_is_valid_hpp
#define ca_vocabulary_unicode_is_valid_hpp

#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/sv/basic.hpp"
#include "2-vocabulary/unicode/find_invalid.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- IS VALID -------------------------------------------------------

		/* @DOC Whether text is well-formed in C's encoding: !find_invalid(text). */
		template <ca::is_character C>
		[[nodiscard]] constexpr auto is_valid(const ca::sv::basic<C> text) noexcept -> bool {
			return !ca::unicode::find_invalid(text).has_value();
		}

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_is_valid_hpp
