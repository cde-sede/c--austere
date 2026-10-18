#ifndef ca_vocabulary_unicode_count_code_points_hpp
#define ca_vocabulary_unicode_count_code_points_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/sv/impl/forward.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"
#include "2-vocabulary/unicode/impl/skip_ascii.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- COUNT CODE POINTS ----------------------------------------------

		/* @DOC Code points in text, each invalid sequence counting as one (the U+FFFD decoding
		   yields): as many as code_points(text) visits. UTF-8 counts ASCII 16 bytes at a time.
		   --BENCH bench/string.cpp count_code_points
		   bench/string.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

		   | Case | clang 18 | gcc 13 | Notes |
		   |---|---|---|---|
		   | count code points of 1 MiB of mixed UTF-8, 20 times | 0.67x | 0.30x | std: mbstowcs(nullptr, ...) |
		   BENCH-- */
		template <ca::is_character C>
		[[nodiscard]] constexpr auto count_code_points(const ca::sv::basic<C> text) noexcept -> ca::usz {

			if constexpr (sizeof(C) == 4U)
				return text.size();
			const C* const end = text.end();
			const C* at = text.begin();
			ca::usz count = 0U;
			while (at != end) {
				if constexpr (sizeof(C) == 1U) {
					const C* const skipped = ca::unicode::impl::skip_ascii(at, end);
					count += static_cast<ca::usz>(skipped - at);
					at = skipped;
					if (at == end)
						break;
				}
				at += ca::unicode::impl::decode_at(at, end).length();
				++count;
			}
			return count;
		}

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_count_code_points_hpp
