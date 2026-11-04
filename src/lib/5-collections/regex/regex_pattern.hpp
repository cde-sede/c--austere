#ifndef ca_collections_regex_regex_pattern_hpp
#define ca_collections_regex_regex_pattern_hpp

#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "5-collections/regex/impl/regex_errors.hpp"
#include "5-collections/regex/impl/regex_parser.hpp"
#include "5-collections/regex/impl/regex_validator.hpp"
#include "5-collections/regex/regex_error.hpp"
#include "5-collections/regex/regex_flags.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX PATTERN ------------------------------------------------------

	/* @DOC The text of a regular expression, what basic_regex::make compiles. From a literal it
	   is checked when it compiles: a pattern invalid both plainly and as verbose (flags given at
	   runtime can turn verbose on) is a compile error naming the problem, such as
	   `regex_has_unbalanced_paren`. Text known only at runtime goes in explicitly,
	   `ca::regex_pattern{text}`, and is checked by make. */
	class regex_pattern final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::regex_pattern;


		private:

			// -- private members --------------------------------------------

			ca::sv::str _text;


		public:

			// -- public lifecycle -------------------------------------------

			consteval regex_pattern(const char* const text) noexcept
			: _text{text} {

				const ca::optional<ca::regex_error> plain = this->check(ca::regex_flags::none);
				if (!plain)
					return;
				const ca::optional<ca::regex_error> verbose = this->check(ca::regex_flags::verbose);
				if (!verbose)
					return;
				ca::impl::regex_reject(plain->what());
			}

			/* @DOC Runtime text, checked by make. */
			constexpr explicit regex_pattern(const ca::sv::str text) noexcept
			: _text{text} {
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto text(void) const noexcept -> ca::sv::str {
				return _text;
			}


			// -- methods ----------------------------------------------------

			/* @DOC The first syntax error under flags (what make would fail with, short of
			   memory and program size), or nothing. Python's re.error: kind, position, message.
			   constexpr. */
			[[nodiscard]] constexpr auto check(const ca::regex_flags flags = ca::regex_flags::none) const noexcept -> ca::optional<ca::regex_error> {

				ca::impl::regex_validator validator{};
				ca::impl::regex_parser<ca::impl::regex_validator> parser{_text, validator};
				return parser.run(flags);
			}

	}; // class regex_pattern

} // namespace ca

#endif // ca_collections_regex_regex_pattern_hpp
