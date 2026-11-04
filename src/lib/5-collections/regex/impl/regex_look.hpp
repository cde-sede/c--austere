#ifndef ca_collections_regex_impl_regex_look_hpp
#define ca_collections_regex_impl_regex_look_hpp

#include "1-primitives/integers/u8.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX LOOK ---------------------------------------------------------

	namespace impl {

		/* @DOC The zero-width assertions: `\A` begin_text, `\Z` end_text, `$` end_text_or_newline
		   (before a final '\n' too), `^` and `$` under multiline begin_line and end_line, `\b`
		   and `\B` over Unicode or (ascii) ASCII word characters. */
		enum class regex_look : ca::u8 {
			begin_text,
			end_text,
			end_text_or_newline,
			begin_line,
			end_line,
			word,
			not_word,
			word_ascii,
			not_word_ascii,
		};

		/* @DOC The named classes `\w \d \s`, Unicode or (ascii) ASCII. */
		enum class regex_class : ca::u8 {
			word,
			digit,
			space,
		};

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_look_hpp
