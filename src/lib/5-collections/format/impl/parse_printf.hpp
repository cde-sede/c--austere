#ifndef ca_collections_format_impl_parse_printf_hpp
#define ca_collections_format_impl_parse_printf_hpp

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


	// -- PARSE PRINTF -------------------------------------------------------

	namespace impl {

		[[nodiscard]] consteval auto is_length_modifier(const char c) noexcept -> bool {
			return c == 'h' || c == 'l' || c == 'j' || c == 'z' || c == 't' || c == 'L' || c == 'q';
		}

		/* @DOC Spec of the conversion whose '%' precedes text[at]; leaves at past its letter. Length
		   modifiers are read and ignored: the argument's type is known. */
		consteval auto parse_printf_spec(const char* const text, ca::usz& at, const ca::impl::display_kind kind) noexcept
		-> ca::impl::format_spec {

			ca::impl::format_spec spec{};
			bool left = false;
			for (;; ++at) {
				const char c = text[at];
				if (c == '-')
					left = true;
				else if (c == '+')
					spec._sign = '+';
				else if (c == ' ') {
					if (spec._sign == '\0')
						spec._sign = ' ';
				}
				else if (c == '#')
					spec._alternate = true;
				else if (c == '0')
					spec._zero = true;
				else
					break;
			}
			if (text[at] == '*')
				ca::impl::printf_star_width_or_precision_is_unsupported();
			spec._width = static_cast<ca::u16>(ca::impl::parse_number(text, at, ca::impl::max_width));
			if (text[at] == '.') {
				++at;
				if (text[at] == '*')
					ca::impl::printf_star_width_or_precision_is_unsupported();
				spec._precision = static_cast<ca::i16>(ca::impl::parse_number(text, at, ca::impl::max_precision));
			}
			while (ca::impl::is_length_modifier(text[at]))
				++at;
			const char conversion = text[at];
			if (conversion == '\0')
				ca::impl::format_placeholder_is_malformed();
			++at;
			switch (conversion) {
				case 'd': case 'i':
					spec._type = 'd';
					break;
				case 'u':
					spec._type = 'd';
					spec._wrap = true;
					break;
				case 'x': case 'X': case 'o': case 'b': case 'B':
					spec._type = conversion;
					spec._wrap = true;
					break;
				case 's':
					spec._type = '\0';
					break;
				case 'c': case 'p': case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A':
					spec._type = conversion;
					break;
				case 'n':
					ca::impl::printf_n_conversion_is_unsupported();
					break;
				default:
					ca::impl::printf_conversion_is_unknown();
					break;
			}
			const bool integral = kind == ca::impl::display_kind::signed_integer || kind == ca::impl::display_kind::unsigned_integer
				|| kind == ca::impl::display_kind::boolean || kind == ca::impl::display_kind::character;
			if (integral && spec._precision >= 0)
				spec._zero = false;
			const bool numeric = integral || kind == ca::impl::display_kind::floating || kind == ca::impl::display_kind::pointer;
			if (left) {
				spec._align = '<';
				 spec._zero = false;
			}
			else if (!(spec._zero && numeric))
				spec._align = '>';
			if (kind == ca::impl::display_kind::custom && spec._width == 0U && !spec._zero)
				spec._align = '\0';
			return spec;
		}

		/* @DOC Splits a printf format into count + 1 literal pieces and count specs, each checked
		   against its argument's kind. */
		consteval auto parse_printf(const char* const text, const ca::impl::display_kind* const kinds, const ca::usz count, ca::impl::format_piece* const pieces, ca::impl::format_spec* const specs) noexcept -> void {

			const ca::usz length = ca::string_length(text);
			if (length > 0xFFFFFFFFU)
				ca::impl::format_is_too_long();
			ca::usz at = 0U;
			ca::usz start = 0U;
			ca::usz index = 0U;
			char escape = '\0';
			while (at < length) {
				if (text[at] != '%') {
					++at;
					continue;
				}
				if (text[at + 1U] == '%') {
					escape = '%';
					at += 2U;
					continue;
				}
				if (index == count)
					ca::impl::format_has_more_placeholders_than_arguments();
				pieces[index] = ca::impl::format_piece{static_cast<ca::u32>(start), static_cast<ca::u32>(at - start), escape};
				++at;
				specs[index] = ca::impl::parse_printf_spec(text, at, kinds[index]);
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

#endif // ca_collections_format_impl_parse_printf_hpp
