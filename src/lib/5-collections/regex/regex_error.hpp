#ifndef ca_collections_regex_regex_error_hpp
#define ca_collections_regex_regex_error_hpp

#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX ERROR --------------------------------------------------------

	/* @DOC Why a pattern does not compile and where: kind, the offset of the code unit it was
	   found at, and Python's message for it (re.error's msg and pos). The constructs a
	   linear-time engine cannot run (backreferences, lookaround, conditionals, atomic groups,
	   possessive repeats) are errors of their own kind. */
	class regex_error final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::regex_error;


		public:

			// -- public types -----------------------------------------------

			enum class kind : ca::u8 {
				missing_paren,
				unbalanced_paren,
				unterminated_set,
				bad_range,
				nothing_to_repeat,
				multiple_repeat,
				min_over_max,
				repeat_too_large,
				bad_escape,
				trailing_backslash,
				bad_code_point,
				missing_group_name,
				bad_group_name,
				duplicate_group_name,
				unknown_extension,
				bad_flags,
				global_flags_not_at_start,
				ascii_and_unicode,
				unterminated_comment,
				backreference,
				lookaround,
				conditional,
				atomic,
				named_character,
				invalid_utf8,
				too_deep,
				too_many_groups,
				too_large,
			};


		private:

			// -- private members --------------------------------------------

			ca::usz     _position;
			self::kind  _kind;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr regex_error(const self::kind kind, const ca::usz position) noexcept
			: _position{position}, _kind{kind} {
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto what(void) const noexcept -> self::kind {
				return _kind;
			}

			/* @DOC Offset in the pattern, in code units. */
			[[nodiscard]] constexpr auto position(void) const noexcept -> ca::usz {
				return _position;
			}

			/* @DOC A literal: Python's wording where re has one. */
			[[nodiscard]] constexpr auto message(void) const noexcept -> ca::sv::str {
				return self::message_of(_kind);
			}

			[[nodiscard]] static constexpr auto message_of(const self::kind kind) noexcept -> ca::sv::str {

				switch (kind) {
					case self::kind::missing_paren:
						return "missing ), unterminated subpattern";
					case self::kind::unbalanced_paren:
						return "unbalanced parenthesis";
					case self::kind::unterminated_set:
						return "unterminated character set";
					case self::kind::bad_range:
						return "bad character range";
					case self::kind::nothing_to_repeat:
						return "nothing to repeat";
					case self::kind::multiple_repeat:
						return "multiple repeat";
					case self::kind::min_over_max:
						return "min repeat greater than max repeat";
					case self::kind::repeat_too_large:
						return "the repetition number is too large";
					case self::kind::bad_escape:
						return "bad escape";
					case self::kind::trailing_backslash:
						return "bad escape (end of pattern)";
					case self::kind::bad_code_point:
						return "escape out of the Unicode range or a surrogate";
					case self::kind::missing_group_name:
						return "missing group name";
					case self::kind::bad_group_name:
						return "bad character in group name";
					case self::kind::duplicate_group_name:
						return "redefinition of group name";
					case self::kind::unknown_extension:
						return "unknown extension";
					case self::kind::bad_flags:
						return "bad inline flags";
					case self::kind::global_flags_not_at_start:
						return "global flags not at the start of the expression";
					case self::kind::ascii_and_unicode:
						return "ASCII and UNICODE flags are incompatible";
					case self::kind::unterminated_comment:
						return "missing ), unterminated comment";
					case self::kind::backreference:
						return "backreferences are not supported (linear-time engine)";
					case self::kind::lookaround:
						return "lookaround assertions are not supported (linear-time engine)";
					case self::kind::conditional:
						return "conditional groups are not supported (linear-time engine)";
					case self::kind::atomic:
						return "atomic groups and possessive repeats are not supported";
					case self::kind::named_character:
						return "named characters \\N{...} are not supported";
					case self::kind::invalid_utf8:
						return "pattern is not valid UTF-8";
					case self::kind::too_deep:
						return "pattern nests groups too deeply";
					case self::kind::too_many_groups:
						return "too many groups";
					case self::kind::too_large:
						return "pattern too large";
				}
				return "invalid pattern";
			}

	}; // class regex_error

} // namespace ca

#endif // ca_collections_regex_regex_error_hpp
