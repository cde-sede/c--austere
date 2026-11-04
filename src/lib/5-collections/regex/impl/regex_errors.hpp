#ifndef ca_collections_regex_impl_regex_errors_hpp
#define ca_collections_regex_impl_regex_errors_hpp

#include "5-collections/regex/regex_error.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX ERRORS -------------------------------------------------------

	/* @DOC Never defined: a literal pattern is checked in a consteval constructor, where calling
	   one of these is a compile error naming the problem. */
	namespace impl {

		auto regex_has_missing_paren(void) noexcept -> void;

		auto regex_has_unbalanced_paren(void) noexcept -> void;

		auto regex_has_unterminated_set(void) noexcept -> void;

		auto regex_has_bad_range(void) noexcept -> void;

		auto regex_has_nothing_to_repeat(void) noexcept -> void;

		auto regex_has_multiple_repeat(void) noexcept -> void;

		auto regex_has_min_over_max(void) noexcept -> void;

		auto regex_has_repeat_too_large(void) noexcept -> void;

		auto regex_has_bad_escape(void) noexcept -> void;

		auto regex_has_trailing_backslash(void) noexcept -> void;

		auto regex_has_bad_code_point(void) noexcept -> void;

		auto regex_has_missing_group_name(void) noexcept -> void;

		auto regex_has_bad_group_name(void) noexcept -> void;

		auto regex_has_duplicate_group_name(void) noexcept -> void;

		auto regex_has_unknown_extension(void) noexcept -> void;

		auto regex_has_bad_flags(void) noexcept -> void;

		auto regex_has_global_flags_not_at_start(void) noexcept -> void;

		auto regex_has_ascii_and_unicode(void) noexcept -> void;

		auto regex_has_unterminated_comment(void) noexcept -> void;

		auto regex_backreferences_are_unsupported(void) noexcept -> void;

		auto regex_lookaround_is_unsupported(void) noexcept -> void;

		auto regex_conditionals_are_unsupported(void) noexcept -> void;

		auto regex_atomic_groups_are_unsupported(void) noexcept -> void;

		auto regex_named_characters_are_unsupported(void) noexcept -> void;

		auto regex_is_not_utf8(void) noexcept -> void;

		auto regex_nests_too_deeply(void) noexcept -> void;

		auto regex_has_too_many_groups(void) noexcept -> void;

		auto regex_is_too_large(void) noexcept -> void;

		/* @DOC Calls the function naming what: a compile error in constant evaluation. */
		consteval auto regex_reject(const ca::regex_error::kind what) noexcept -> void {

			using kind = ca::regex_error::kind;
			switch (what) {
				case kind::missing_paren:
					ca::impl::regex_has_missing_paren();
					break;
				case kind::unbalanced_paren:
					ca::impl::regex_has_unbalanced_paren();
					break;
				case kind::unterminated_set:
					ca::impl::regex_has_unterminated_set();
					break;
				case kind::bad_range:
					ca::impl::regex_has_bad_range();
					break;
				case kind::nothing_to_repeat:
					ca::impl::regex_has_nothing_to_repeat();
					break;
				case kind::multiple_repeat:
					ca::impl::regex_has_multiple_repeat();
					break;
				case kind::min_over_max:
					ca::impl::regex_has_min_over_max();
					break;
				case kind::repeat_too_large:
					ca::impl::regex_has_repeat_too_large();
					break;
				case kind::bad_escape:
					ca::impl::regex_has_bad_escape();
					break;
				case kind::trailing_backslash:
					ca::impl::regex_has_trailing_backslash();
					break;
				case kind::bad_code_point:
					ca::impl::regex_has_bad_code_point();
					break;
				case kind::missing_group_name:
					ca::impl::regex_has_missing_group_name();
					break;
				case kind::bad_group_name:
					ca::impl::regex_has_bad_group_name();
					break;
				case kind::duplicate_group_name:
					ca::impl::regex_has_duplicate_group_name();
					break;
				case kind::unknown_extension:
					ca::impl::regex_has_unknown_extension();
					break;
				case kind::bad_flags:
					ca::impl::regex_has_bad_flags();
					break;
				case kind::global_flags_not_at_start:
					ca::impl::regex_has_global_flags_not_at_start();
					break;
				case kind::ascii_and_unicode:
					ca::impl::regex_has_ascii_and_unicode();
					break;
				case kind::unterminated_comment:
					ca::impl::regex_has_unterminated_comment();
					break;
				case kind::backreference:
					ca::impl::regex_backreferences_are_unsupported();
					break;
				case kind::lookaround:
					ca::impl::regex_lookaround_is_unsupported();
					break;
				case kind::conditional:
					ca::impl::regex_conditionals_are_unsupported();
					break;
				case kind::atomic:
					ca::impl::regex_atomic_groups_are_unsupported();
					break;
				case kind::named_character:
					ca::impl::regex_named_characters_are_unsupported();
					break;
				case kind::invalid_utf8:
					ca::impl::regex_is_not_utf8();
					break;
				case kind::too_deep:
					ca::impl::regex_nests_too_deeply();
					break;
				case kind::too_many_groups:
					ca::impl::regex_has_too_many_groups();
					break;
				case kind::too_large:
					ca::impl::regex_is_too_large();
					break;
			}
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_errors_hpp
