#ifndef ca_vocabulary_unicode_transcoded_size_hpp
#define ca_vocabulary_unicode_transcoded_size_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/sv/basic.hpp"
#include "2-vocabulary/unicode/encoded_size.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"
#include "2-vocabulary/unicode/impl/skip_ascii.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- TRANSCODED SIZE ------------------------------------------------

		/* @DOC Code units transcode(text, ...) writes in To's encoding, invalid sequences counting
		   as U+FFFD: the exact output size to allocate. */
		template <ca::is_character To, ca::is_character From>
		[[nodiscard]] constexpr auto transcoded_size(const ca::sv::basic<From> text) noexcept -> ca::usz {

			const From* const end = text.end();
			const From* at = text.begin();
			ca::usz size = 0U;
			while (at != end) {
				if constexpr (sizeof(From) == 1U) {
					const From* const skipped = ca::unicode::impl::skip_ascii(at, end);
					size += static_cast<ca::usz>(skipped - at);
					at = skipped;
					if (at == end)
						break;
				}
				const ca::unicode::decoded point = ca::unicode::impl::decode_at(at, end);
				size += ca::unicode::encoded_size<To>(point.code_point());
				at += point.length();
			}
			return size;
		}

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_transcoded_size_hpp
