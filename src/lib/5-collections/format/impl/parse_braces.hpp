#ifndef ca_collections_format_impl_parse_braces_hpp
#define ca_collections_format_impl_parse_braces_hpp

#include "1-primitives/integers/i16.hpp"
#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/string_length.hpp"
#include "5-collections/format/impl/check_spec.hpp"
#include "5-collections/format/impl/display_kind.hpp"
#include "5-collections/format/impl/format_errors.hpp"
#include "5-collections/format/impl/format_piece.hpp"
#include "5-collections/format/impl/format_spec.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- PARSE BRACES -------------------------------------------------------

	namespace impl {

		[[nodiscard]] consteval auto is_align(const char c) noexcept -> bool {
			return c == '<' || c == '>' || c == '^';
		}

		/* @DOC Spec of the placeholder whose '{' precedes text[at]; leaves at past its '}'. */
		consteval auto parse_brace_spec(const char* const text, ca::usz& at) noexcept -> ca::impl::format_spec {

			ca::impl::format_spec spec{};
			if (text[at] == ':')
				++at;
			if (text[at] != '\0' && text[at] != '}' && ca::impl::is_align(text[at + 1U])) {
				 spec._fill = text[at];
				spec._align = text[at + 1U];
				at += 2U;
			}
			else if (ca::impl::is_align(text[at]))
				spec._align = text[at++];
			if (text[at] == '+' || text[at] == ' ')
				spec._sign = text[at++];
			else if (text[at] == '-')
				++at;
			if (text[at] == '#') {
				spec._alternate = true;
				++at;
			}
			if (text[at] == '0') {
				spec._zero = true;
				++at;
			}
			spec._width = static_cast<ca::u16>(ca::impl::parse_number(text, at, ca::impl::max_width));
			if (text[at] == '.') {
				++at;
				spec._precision = static_cast<ca::i16>(ca::impl::parse_number(text, at, ca::impl::max_precision));
			}
			if (text[at] != '}' && text[at] != '\0')
				spec._type = text[at++];
			if (text[at] != '}')
				ca::impl::format_placeholder_is_malformed();
			++at;
			return spec;
		}

		/* @DOC Splits a {}-format into count + 1 literal pieces and count specs, each checked against
		   its argument's kind. */
		consteval auto parse_braces(const char* const text, const ca::impl::display_kind* const kinds, const ca::usz count, ca::impl::format_piece* const pieces, ca::impl::format_spec* const specs) noexcept -> void {

			const ca::usz length = ca::string_length(text);
			if (length > 0xFFFFFFFFU)
				ca::impl::format_is_too_long();
			ca::usz at = 0U;
			ca::usz start = 0U;
			ca::usz index = 0U;
			char escape = '\0';
			while (at < length) {
				const char c = text[at];
				if ((c == '{' || c == '}') && text[at + 1U] == c) {
					escape = '{';
					at += 2U;
					continue;
				}
				if (c == '}')
					ca::impl::format_has_unmatched_closing_brace();
				if (c != '{') {
					++at;
					continue;
				}
				if (index == count)
					ca::impl::format_has_more_placeholders_than_arguments();
				pieces[index] = ca::impl::format_piece{static_cast<ca::u32>(start), static_cast<ca::u32>(at - start), escape};
				++at;
				specs[index] = ca::impl::parse_brace_spec(text, at);
				ca::impl::check_spec(specs[index], kinds[index]);
				++index;
				 start = at;
				escape = '\0';
			}
			if (index != count)
				ca::impl::format_has_fewer_placeholders_than_arguments();
			pieces[count] = ca::impl::format_piece{static_cast<ca::u32>(start), static_cast<ca::u32>(length - start), escape};
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_format_impl_parse_braces_hpp
