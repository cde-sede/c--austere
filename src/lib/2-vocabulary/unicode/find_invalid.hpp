#ifndef ca_vocabulary_unicode_find_invalid_hpp
#define ca_vocabulary_unicode_find_invalid_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/basic.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"
#include "2-vocabulary/unicode/impl/skip_ascii.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- FIND INVALID ---------------------------------------------------

		/* @DOC Index of the first code unit of text that starts an invalid sequence (overlong,
		   surrogate, past U+10FFFF, truncated, lone continuation), or nothing when text is
		   well-formed. UTF-8 skips ASCII 16 bytes at a time. */
		template <ca::is_character C>
		[[nodiscard]] constexpr auto find_invalid(const ca::sv::basic<C> text) noexcept -> ca::optional<ca::usz> {

			const C* const begin = text.begin();
			const C* const end = text.end();
			const C* at = begin;
			while (at != end) {
				if constexpr (sizeof(C) == 1U) {
					at = ca::unicode::impl::skip_ascii(at, end);
					if (at == end)
						break;
				}
				const ca::unicode::decoded point = ca::unicode::impl::decode_at(at, end);
				if (!point.valid())
					return static_cast<ca::usz>(at - begin);
				at += point.length();
			}
			return ca::nullopt;
		}

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_find_invalid_hpp
