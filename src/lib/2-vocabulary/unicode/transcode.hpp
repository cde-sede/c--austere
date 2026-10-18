#ifndef ca_vocabulary_unicode_transcode_hpp
#define ca_vocabulary_unicode_transcode_hpp

#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"
#include "1-primitives/traits/is_constant_evaluated.hpp"
#include "2-vocabulary/sv/basic.hpp"
#include "2-vocabulary/unicode/encode.hpp"
#include "2-vocabulary/unicode/encoded_size.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"
#include "2-vocabulary/unicode/impl/skip_ascii.hpp"
#include "2-vocabulary/unicode/transcoded.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- TRANSCODE ------------------------------------------------------

		/* @DOC Converts text from its encoding to To's, into the capacity units at out, and stops
		   before the first code point that does not fit (never splits one). Invalid sequences
		   become U+FFFD, so the output is always well-formed: transcoding to the same encoding
		   sanitizes. Size the output with transcoded_size. ASCII from UTF-8 is widened 16 bytes
		   at a time without decoding.
		   --BENCH bench/string.cpp transcode
		   bench/string.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

		   | Case | clang 18 | gcc 13 | Notes |
		   |---|---|---|---|
		   | UTF-8 to UTF-32, 1 MiB of mixed text, 20 times | 0.66x | 0.96x | std: mbstowcs |
		   BENCH-- */
		template <ca::is_character To, ca::is_character From>
		constexpr auto transcode(const ca::sv::basic<From> text, To* const out, const ca::usz capacity) noexcept -> ca::unicode::transcoded {

			const From* const begin = text.begin();
			const From* const end = text.end();
			const From* at = begin;
			To* to = out;
			To* const limit = out + capacity;
			if constexpr (sizeof(From) == 1U) {
				if (!ca::is_constant_evaluated()) {
					while (end - at >= 16 && limit - to >= 16 && ca::unicode::impl::is_ascii_block(at)) {
						for (ca::usz i = 0U; i < 16U; ++i)
							to[i] = static_cast<To>(at[i]);
						at += 16;
						to += 16;
					}
				}
			}
			while (at != end && to != limit) {
				if constexpr (sizeof(From) == 1U) {
					const ca::usz room = static_cast<ca::usz>(end - at) < static_cast<ca::usz>(limit - to) ? static_cast<ca::usz>(end - at) : static_cast<ca::usz>(limit - to);
					ca::usz run = 0U;
					while (run < room && static_cast<ca::u8>(at[run]) < 0x80U) {
						to[run] = static_cast<To>(at[run]);
						++run;
					}
					at += run;
					to += run;
					if (at == end || to == limit)
						break;
				}
				const ca::unicode::decoded point = ca::unicode::impl::decode_at(at, end);
				if (static_cast<ca::usz>(limit - to) < ca::unicode::encoded_size<To>(point.code_point()))
					break;
				to += ca::unicode::encode(point.code_point(), to);
				at += point.length();
			}
			return ca::unicode::transcoded{static_cast<ca::usz>(at - begin), static_cast<ca::usz>(to - out)};
		}

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_transcode_hpp
