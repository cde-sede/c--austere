#ifndef ca_vocabulary_unicode_impl_char_record_hpp
#define ca_vocabulary_unicode_impl_char_record_hpp

#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/u16.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- CHAR RECORD ----------------------------------------------------

		namespace impl {

			/* @DOC What the tables hold for a group of code points sharing properties: flags, the
			   simple case mappings as deltas (code point + delta) and, with SPECIAL, the index of
			   the full mappings (several code points) in special_cases. */
			struct char_record final {

					using self = ca::unicode::impl::char_record;

					static constexpr ca::u16 ALPHA          = 1U << 0U;
					static constexpr ca::u16 DECIMAL        = 1U << 1U;
					static constexpr ca::u16 DIGIT          = 1U << 2U;
					static constexpr ca::u16 NUMERIC        = 1U << 3U;
					static constexpr ca::u16 SPACE          = 1U << 4U;
					static constexpr ca::u16 LINE_BREAK     = 1U << 5U;
					static constexpr ca::u16 UPPER          = 1U << 6U;
					static constexpr ca::u16 LOWER          = 1U << 7U;
					static constexpr ca::u16 TITLE          = 1U << 8U;
					static constexpr ca::u16 CASED          = 1U << 9U;
					static constexpr ca::u16 CASE_IGNORABLE = 1U << 10U;
					static constexpr ca::u16 PRINTABLE      = 1U << 11U;
					static constexpr ca::u16 ID_START       = 1U << 12U;
					static constexpr ca::u16 ID_CONTINUE    = 1U << 13U;
					static constexpr ca::u16 SPECIAL        = 1U << 14U;

					ca::u16 _flags;
					ca::i32 _upper;
					ca::i32 _lower;
					ca::i32 _title;
					ca::i32 _fold;
					ca::u16 _special;

			}; // struct char_record

			/* @DOC Full mappings of one code point, each up to 3 code points (0-padded). */
			struct special_case final {

					using self = ca::unicode::impl::special_case;

					char32_t _upper[3];
					char32_t _lower[3];
					char32_t _title[3];
					char32_t _fold[3];

			}; // struct special_case

		} // namespace impl

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_impl_char_record_hpp
