#ifndef ca_collections_regex_impl_regex_replacement_hpp
#define ca_collections_regex_impl_regex_replacement_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "2-vocabulary/unicode/decoded.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"
#include "2-vocabulary/unicode/properties_of.hpp"
#include "5-collections/regex/impl/regex_names.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX REPLACEMENT --------------------------------------------------

	namespace impl {

		[[nodiscard]] constexpr auto regex_template_escape(const char c) noexcept -> char32_t {
			switch (c) {
				case 'a':
					return U'\a';
				case 'b':
					return U'\b';
				case 'f':
					return U'\f';
				case 'n':
					return U'\n';
				case 'r':
					return U'\r';
				case 't':
					return U'\t';
				case 'v':
					return U'\v';
				case '\\':
					return U'\\';
				default:
					return U'\0';
			}
		}

		[[nodiscard]] constexpr auto regex_is_identifier(const ca::sv::str name) noexcept -> bool {

			ca::usz at = 0U;
			while (at < name.size()) {
				const ca::unicode::decoded point = ca::unicode::impl::decode_at(name.data() + at, name.data() + name.size());
				if (!point.valid())
					return false;
				const ca::unicode::properties properties = ca::unicode::properties_of(point.code_point());
				if (at == 0U ? !properties.is_identifier_start() : !properties.is_identifier_continue())
					return false;
				at += point.length();
			}
			return at != 0U;
		}

		/* @DOC Walks a replacement template as Python's re.sub reads it: text to sink.text(view),
		   escapes (\n, \t, ..., octal \0 \12 \123) to sink.point(code point), \g<name>,
		   \g<number>, \1 to \99 to sink.group(index). An unknown escape of an ASCII letter, a
		   group past groups or an unknown name is an error: its message, a literal; nothing
		   when the template is fine. Other escapes (\&) stay as written. */
		template <typename Sink>
		[[nodiscard]] constexpr auto regex_expand(const ca::sv::str replacement, const ca::u32 groups, const ca::impl::regex_names& names, Sink& sink) noexcept
		-> ca::optional<ca::sv::str> {

			const char* const text = replacement.data();
			const ca::usz size = replacement.size();
			ca::usz start = 0U;
			ca::usz at = 0U;
			const auto is_octal = [](const char c) noexcept -> bool {
				return c >= '0' && c <= '7';
			};
			const auto is_digit = [](const char c) noexcept -> bool {
				return c >= '0' && c <= '9';
			};
			while (at < size) {
				if (text[at] != '\\') {
					++at;
					continue;
				}
				if (at > start)
					sink.text(ca::sv::str{text + start, at - start});
				if (at + 1U == size)
					return ca::sv::str{"bad escape (end of pattern)"};
				const char c = text[at + 1U];
				at += 2U;
				if (c == 'g') {
					if (at == size || text[at] != '<')
						return ca::sv::str{"missing <"};
					const ca::usz name_at = ++at;
					while (at < size && text[at] != '>')
						++at;
					if (at == size)
						return ca::sv::str{"missing >, unterminated name"};
					const ca::sv::str name{text + name_at, at - name_at};
					++at;
					if (name.empty())
						return ca::sv::str{"missing group name"};
					ca::u32 index = 0U;
					bool number = true;
					for (const char unit : name) {
						if (!is_digit(unit))
							number = false;
						else if (index <= groups)
							index = index * 10U + static_cast<ca::u32>(unit - '0');
					}
					if (!number) {
						if (!ca::impl::regex_is_identifier(name))
							return ca::sv::str{"bad character in group name"};
						const ca::optional<ca::u32> named = names.find(name);
						if (!named)
							return ca::sv::str{"unknown group name"};
						index = *named;
					}
					if (index > groups)
						return ca::sv::str{"invalid group reference"};
					sink.group(index);
				}
				else if (c == '0') {
					ca::u32 value = 0U;
					for (ca::usz i = 0U; i < 2U && at < size && is_octal(text[at]); ++i)
						value = value * 8U + static_cast<ca::u32>(text[at++] - '0');
					sink.point(static_cast<char32_t>(value & 0xFFU));
				}
				else if (is_digit(c)) {
					ca::u32 value = static_cast<ca::u32>(c - '0');
					if (at < size && is_digit(text[at])) {
						if (is_octal(c) && is_octal(text[at]) && at + 1U < size && is_octal(text[at + 1U])) {
							value = value * 64U + static_cast<ca::u32>(text[at] - '0') * 8U + static_cast<ca::u32>(text[at + 1U] - '0');
							at += 2U;
							if (value > 0377U)
								return ca::sv::str{"octal escape value outside of range 0-0o377"};
							sink.point(static_cast<char32_t>(value));
							start = at;
							continue;
						}
						value = value * 10U + static_cast<ca::u32>(text[at++] - '0');
					}
					if (value > groups)
						return ca::sv::str{"invalid group reference"};
					sink.group(value);
				}
				else if (ca::impl::regex_template_escape(c) != U'\0')
					sink.point(ca::impl::regex_template_escape(c));
				else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
					return ca::sv::str{"bad escape"};
				else {
					start = at - 2U;
					continue;
				}
				start = at;
			}
			if (at > start)
				sink.text(ca::sv::str{text + start, at - start});
			return ca::nullopt;
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_replacement_hpp
