#ifndef ca_vocabulary_unicode_decode_hpp
#define ca_vocabulary_unicode_decode_hpp

#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/basic.hpp"
#include "2-vocabulary/unicode/decoded.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- DECODE ---------------------------------------------------------

		/* @DOC The first code point of text in C's encoding (UTF-8, UTF-16 or UTF-32 by the unit
		   size), or nothing when text is empty. */
		template <ca::is_character C>
		[[nodiscard]] constexpr auto decode(const ca::sv::basic<C> text) noexcept -> ca::optional<ca::unicode::decoded> {

			if (text.empty())
				return ca::nullopt;
			return ca::unicode::impl::decode_at(text.begin(), text.end());
		}

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_decode_hpp
