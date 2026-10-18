#ifndef ca_vocabulary_unicode_properties_hpp
#define ca_vocabulary_unicode_properties_hpp

#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/u16.hpp"
#include "2-vocabulary/unicode/impl/char_record.hpp"
#include "2-vocabulary/unicode/impl/tables.hpp"
#include "2-vocabulary/unicode/mapping.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- PROPERTIES -----------------------------------------------------

		/* @DOC The character database entry of one code point (see properties_of): classes as
		   Python's str methods define them, and case mappings, simple (one code point) or full
		   (mapping). Tables generated from the running Python and the UCD of its Unicode
		   version (make unicode). */
		class properties final {


			private:

				// -- private types ------------------------------------------

				using self = ca::unicode::properties;

				using record = ca::unicode::impl::char_record;


				// -- private members ----------------------------------------

				const record* _record;
				char32_t _point;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr properties(const ca::unicode::impl::char_record& entry, const char32_t point) noexcept
				: _record{&entry}, _point{point} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto code_point(void) const noexcept -> char32_t {
					return _point;
				}

				/* @DOC Letter (categories Lm Lt Lu Ll Lo): str.isalpha. */
				[[nodiscard]] constexpr auto is_alpha(void) const noexcept -> bool {
					return this->_has(record::ALPHA);
				}

				/* @DOC Decimal digit (Nd): str.isdecimal. */
				[[nodiscard]] constexpr auto is_decimal(void) const noexcept -> bool {
					return this->_has(record::DECIMAL);
				}

				/* @DOC Digit value (decimal, superscripts, ...): str.isdigit. */
				[[nodiscard]] constexpr auto is_digit(void) const noexcept -> bool {
					return this->_has(record::DIGIT);
				}

				/* @DOC Numeric value (digits, fractions, numerals): str.isnumeric. */
				[[nodiscard]] constexpr auto is_numeric(void) const noexcept -> bool {
					return this->_has(record::NUMERIC);
				}

				/* @DOC Alpha, decimal, digit or numeric: str.isalnum. */
				[[nodiscard]] constexpr auto is_alnum(void) const noexcept -> bool {
					return this->_has(record::ALPHA | record::DECIMAL | record::DIGIT | record::NUMERIC);
				}

				/* @DOC White space: str.isspace, str.split(). */
				[[nodiscard]] constexpr auto is_space(void) const noexcept -> bool {
					return this->_has(record::SPACE);
				}

				/* @DOC Line boundary: str.splitlines. */
				[[nodiscard]] constexpr auto is_line_break(void) const noexcept -> bool {
					return this->_has(record::LINE_BREAK);
				}

				[[nodiscard]] constexpr auto is_upper(void) const noexcept -> bool {
					return this->_has(record::UPPER);
				}

				[[nodiscard]] constexpr auto is_lower(void) const noexcept -> bool {
					return this->_has(record::LOWER);
				}

				/* @DOC Titlecase letter (Lt), such as the digraph U+01C5. */
				[[nodiscard]] constexpr auto is_title(void) const noexcept -> bool {
					return this->_has(record::TITLE);
				}

				/* @DOC Has case: upper, lower or title. */
				[[nodiscard]] constexpr auto is_cased(void) const noexcept -> bool {
					return this->_has(record::CASED);
				}

				/* @DOC Skipped when looking for a cased neighbour (apostrophe, period, ...). */
				[[nodiscard]] constexpr auto is_case_ignorable(void) const noexcept -> bool {
					return this->_has(record::CASE_IGNORABLE);
				}

				/* @DOC str.isprintable: not a control, format, separator (but space) or unassigned
				   code point. */
				[[nodiscard]] constexpr auto is_printable(void) const noexcept -> bool {
					return this->_has(record::PRINTABLE);
				}

				/* @DOC May start an identifier (XID_Start and '_'): str.isidentifier. */
				[[nodiscard]] constexpr auto is_identifier_start(void) const noexcept -> bool {
					return this->_has(record::ID_START);
				}

				[[nodiscard]] constexpr auto is_identifier_continue(void) const noexcept -> bool {
					return this->_has(record::ID_CONTINUE);
				}

				/* @DOC Simple uppercase mapping: one code point. */
				[[nodiscard]] constexpr auto to_upper(void) const noexcept -> char32_t {
					return this->_shifted(_record->_upper);
				}

				[[nodiscard]] constexpr auto to_lower(void) const noexcept -> char32_t {
					return this->_shifted(_record->_lower);
				}

				[[nodiscard]] constexpr auto to_title(void) const noexcept -> char32_t {
					return this->_shifted(_record->_title);
				}

				/* @DOC Simple case folding, for caseless comparison. */
				[[nodiscard]] constexpr auto to_fold(void) const noexcept -> char32_t {
					return this->_shifted(_record->_fold);
				}

				/* @DOC Full uppercase mapping: str.upper. */
				[[nodiscard]] constexpr auto upper(void) const noexcept -> ca::unicode::mapping {

					if (this->_has(record::SPECIAL))
						return ca::unicode::mapping{ca::unicode::impl::special_cases[_record->_special]._upper};
					return ca::unicode::mapping{this->to_upper()};
				}

				/* @DOC Full lowercase mapping: str.lower. */
				[[nodiscard]] constexpr auto lower(void) const noexcept -> ca::unicode::mapping {

					if (this->_has(record::SPECIAL))
						return ca::unicode::mapping{ca::unicode::impl::special_cases[_record->_special]._lower};
					return ca::unicode::mapping{this->to_lower()};
				}

				/* @DOC Full titlecase mapping: str.title, str.capitalize. */
				[[nodiscard]] constexpr auto title(void) const noexcept -> ca::unicode::mapping {

					if (this->_has(record::SPECIAL))
						return ca::unicode::mapping{ca::unicode::impl::special_cases[_record->_special]._title};
					return ca::unicode::mapping{this->to_title()};
				}

				/* @DOC Full case folding: str.casefold. */
				[[nodiscard]] constexpr auto fold(void) const noexcept -> ca::unicode::mapping {

					if (this->_has(record::SPECIAL))
						return ca::unicode::mapping{ca::unicode::impl::special_cases[_record->_special]._fold};
					return ca::unicode::mapping{this->to_fold()};
				}


			private:

				// -- private methods ----------------------------------------

				[[nodiscard]] constexpr auto _has(const ca::u16 flags) const noexcept -> bool {
					return (_record->_flags & flags) != 0U;
				}

				[[nodiscard]] constexpr auto _shifted(const ca::i32 delta) const noexcept -> char32_t {
					return static_cast<char32_t>(static_cast<ca::i32>(_point) + delta);
				}

		}; // class properties

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_properties_hpp
